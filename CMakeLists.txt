cmake_minimum_required(VERSION 3.15)

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type")
endif()
set(BUILD_DNG_VALIDATE OFF CACHE BOOL "Build dng_validate executable")
# Declare early so the dng install rules below can test it safely.
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)

project(dng VERSION 1.7.1 DESCRIPTION "DNG SDK shared library" LANGUAGES C CXX)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install prefix" FORCE)
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

include(GNUInstallDirs)
include(ExternalProject)
include(ProcessorCount)

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(PYDNG_PY_PKG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/pydng)
get_filename_component(PYDNG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PyDNGBindings.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PyDNGSources.cmake)

# XMP headers: Adobe code uses #if _MSC_VER <= 1600; undefined _MSC_VER is treated as 0 → broken std::tr1 on MinGW.
function(pydng_windows_gnu_xmp_fix target)
    if(WIN32 AND NOT MSVC)
        target_compile_definitions(${target} PRIVATE _MSC_VER=1920)
    endif()
endfunction()

add_library(dng SHARED ${SRC_FILES})
set_target_properties(dng PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

set(LIBJPEG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg)
set(JXL_BUILD_DIR ${CMAKE_BINARY_DIR}/libjxl-prefix/src/libjxl-build)

target_include_directories(dng PRIVATE
    ${ZLIB_DIR}
    ${CMAKE_INSTALL_PREFIX}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/public/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFilesPlugins/api/source
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/third-party/expat/public/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/third-party/boost
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/public/include/XMPCore/Interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/public/include/client-glue
)
if(WIN32)
    target_include_directories(dng PRIVATE ${LIBJPEG_DIR})
endif()

if(NOT WIN32)
    ProcessorCount(N)
    ExternalProject_Add(
        libjpeg
        SOURCE_DIR ${LIBJPEG_DIR}
        BINARY_DIR ${LIBJPEG_DIR}
        # Invoke via ``sh`` so checkout without +x on ``configure`` (e.g. GitHub Actions) still works.
        CONFIGURE_COMMAND sh ${LIBJPEG_DIR}/configure --prefix=${CMAKE_INSTALL_PREFIX}
        BUILD_COMMAND make -j${N}
    )
endif()

ExternalProject_Add(
    libjxl
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjxl/libjxl
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DBUILD_SHARED_LIBS=OFF
        -DBUILD_TESTING=OFF
        -DJPEGXL_ENABLE_SJPEG=OFF
        -DJPEGXL_ENABLE_EXAMPLES=OFF
        -DJPEGXL_ENABLE_TOOLS=OFF
        -DJPEGXL_ENABLE_BENCHMARK=OFF
        -DJPEGXL_ENABLE_MANPAGES=OFF
        -DJPEGXL_ENABLE_JPEGLI=OFF
        -DBROTLI_BUNDLED_MODE=ON
        -DHWY_ENABLE_INSTALL=OFF
        -DHWY_ENABLE_TESTS=OFF
        -DJPEGXL_STATIC=ON
        -DJPEGXL_FORCE_SYSTEM_BROTLI=OFF
)

if(NOT WIN32)
    add_dependencies(dng libjpeg)
endif()
add_dependencies(dng libjxl)

target_compile_definitions(
    dng PRIVATE
    $<IF:$<CONFIG:Debug>,qDNGDebug=1,qDNGDebug=0>
    qDNGValidate=1
    qDNGUseXMP=1
    qDNGUseLibJPEG=1
    AdobePrivate=1
    BUILDING_XMPCORE_LIB=1
    BUILDING_XMPCORE_AS_STATIC=1
    XMP_StaticBuild=1
    XMP_COMPONENT_INT_NAMESPACE=AdobeXMPCore_Int
    JXL_STATIC_DEFINE=1
)

if(BUILD_DNG_VALIDATE)
    target_compile_definitions(dng PUBLIC qDNGValidateTarget=1)
else()
    target_compile_definitions(dng PUBLIC qDNGValidateTarget=0)
endif()

if(APPLE)
    target_compile_definitions(dng PUBLIC qMacOS=1 MAC_ENV XMP_64=1)
    target_compile_options(dng PUBLIC -Wno-implicit-function-declaration)
    target_link_libraries(dng PRIVATE "-framework CoreServices")
elseif(WIN32)
    target_compile_definitions(dng PUBLIC qWinOS=1 WIN_ENV NOMINMAX UNICODE)
else()
    target_compile_options(dng PUBLIC -fPIC -Wall -Wextra -Wno-implicit-function-declaration)
    target_compile_definitions(dng PUBLIC qLinux=1 UNIX_ENV XMP_64=1)
endif()

pydng_windows_gnu_xmp_fix(dng)

target_link_directories(dng PRIVATE
    ${CMAKE_INSTALL_PREFIX}/lib
    ${JXL_BUILD_DIR}/lib
    ${JXL_BUILD_DIR}/third_party/brotli
    ${JXL_BUILD_DIR}/third_party/highway
)

set(PYDNG_JXL_LIBS
    jxl-static
    jxl_dec-static
    jxl_threads-static
    hwy
    brotlienc-static
    brotlidec-static
    brotlicommon-static
)

if(WIN32)
    target_link_libraries(dng PRIVATE ${PYDNG_JXL_LIBS})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(dng PRIVATE
        jpeg
        jxl
        jxl_threads
        hwy
        brotlienc-static
        brotlidec-static
        brotlicommon-static
    )
else()
    target_link_libraries(dng PRIVATE jpeg jxl-static jxl_threads-static)
endif()

if(BUILD_DNG_VALIDATE)
    set(DNG_VALIDATE_EXTRA
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_globals.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_xmp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_host.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_xmp_sdk.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_memory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_sdk.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_bottlenecks.cpp
    )
    add_executable(dng_validate ${DNG_VALIDATE_SRC} ${DNG_VALIDATE_EXTRA})
    target_compile_definitions(dng_validate PRIVATE
        qDNGValidateTarget=1
        qDNGUseXMP=1
        qDNGUseLibJPEG=1
        qDNGUseLibJXL=1
        qDNGDebug=0
        qDNGValidate=1
        AdobePrivate=1
        BUILDING_XMPCORE_LIB=1
        BUILDING_XMPCORE_AS_STATIC=1
        XMP_StaticBuild=1
        XMP_COMPONENT_INT_NAMESPACE=AdobeXMPCore_Int
    )
    if(WIN32)
        target_compile_definitions(dng_validate PRIVATE qWinOS=1 WIN_ENV NOMINMAX UNICODE)
    endif()
    pydng_windows_gnu_xmp_fix(dng_validate)
    target_include_directories(dng_validate PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/public/include
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/third-party/expat/public/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/third-party/boost
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/public/include/XMPCore/Interfaces
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/public/include/client-glue
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjxl/libjxl/lib/include
    )
    target_link_libraries(dng_validate PRIVATE dng)
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/
        DESTINATION include/dng FILES_MATCHING PATTERN "*.h")
# On Windows the DLL (RUNTIME artifact) defaults to bin/. When building the Python package we need dng.dll
# next to _native.pyd inside pydng/ so Python 3.8+ add_dll_directory() can resolve it at runtime.
if(WIN32)
    if(BUILD_PYTHON_BINDINGS)
        install(TARGETS dng
            RUNTIME DESTINATION pydng
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    else()
        install(TARGETS dng
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
    endif()
else()
    install(TARGETS dng LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
if(BUILD_DNG_VALIDATE)
    install(TARGETS dng_validate DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(BUILD_PYTHON_BINDINGS)
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
    set(PYBIND11_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/pybind11)
    if(NOT EXISTS "${PYBIND11_DIR}/CMakeLists.txt")
        find_package(pybind11 QUIET)
        if(NOT pybind11_FOUND)
            include(FetchContent)
            FetchContent_Declare(pybind11
                GIT_REPOSITORY https://github.com/pybind/pybind11.git
                GIT_TAG v2.11.1
            )
            FetchContent_MakeAvailable(pybind11)
        endif()
    else()
        add_subdirectory(${PYBIND11_DIR} ${CMAKE_BINARY_DIR}/pybind11)
    endif()

    pybind11_add_module(_native ${PYDNG_BINDINGS_SOURCES})
    set_target_properties(_native PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)

    target_include_directories(_native PRIVATE
        ${PYDNG_BINDINGS_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/public/include
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/third-party/expat/public/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/third-party/boost
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/public/include/XMPCore/Interfaces
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/public/include/client-glue
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjxl/libjxl/lib/include
        ${Python3_INCLUDE_DIRS}
    )

    target_compile_definitions(_native PRIVATE
        qDNGDebug=0
        qDNGValidate=1
        qDNGUseXMP=1
        qDNGUseLibJPEG=1
        qDNGUseLibJXL=1
        AdobePrivate=1
        BUILDING_XMPCORE_LIB=1
        BUILDING_XMPCORE_AS_STATIC=1
        XMP_StaticBuild=1
        XMP_COMPONENT_INT_NAMESPACE=AdobeXMPCore_Int
        JXL_STATIC_DEFINE=1
    )
    if(WIN32)
        target_compile_definitions(_native PRIVATE qWinOS=1 WIN_ENV NOMINMAX UNICODE)
    endif()
    pydng_windows_gnu_xmp_fix(_native)

    target_link_libraries(_native PRIVATE dng ${Python3_LIBRARIES})
    target_link_directories(_native PRIVATE ${CMAKE_INSTALL_PREFIX}/lib ${JXL_BUILD_DIR}/lib)

    install(TARGETS _native
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/pydng
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/pydng
    )
    install(FILES
        ${PYDNG_PY_PKG_DIR}/__init__.py
        ${PYDNG_PY_PKG_DIR}/__init__.pyi
        ${PYDNG_PY_PKG_DIR}/_native.pyi
        ${PYDNG_PY_PKG_DIR}/py.typed
        DESTINATION ${CMAKE_INSTALL_PREFIX}/pydng
    )

    if(WIN32)
        add_custom_command(TARGET _native POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/python/pydng
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${PYDNG_PY_PKG_DIR} ${CMAKE_BINARY_DIR}/python/pydng
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:dng> ${CMAKE_BINARY_DIR}/python/pydng/
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:_native> ${CMAKE_BINARY_DIR}/python/pydng/
        )
    else()
        execute_process(
            COMMAND ${Python3_EXECUTABLE} -c "import site; print(site.getusersitepackages())"
            OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(PYTHON_SITE_PACKAGES)
            add_custom_command(TARGET _native POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_SITE_PACKAGES}/pydng
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${PYDNG_PY_PKG_DIR} ${PYTHON_SITE_PACKAGES}/pydng
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:_native> ${PYTHON_SITE_PACKAGES}/pydng/
            )
        endif()
    endif()
endif()
