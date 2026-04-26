cmake_minimum_required(VERSION 3.15)

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type")
endif()
set(BUILD_DNG_VALIDATE OFF CACHE BOOL "Build dng_validate executable")
# Declare early so the dng install rules below can test it safely.
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)
option(BUILD_DNG_LIBRARY "Build DNG SDK shared library" ON)
set(PREBUILT_DNG_PATH "" CACHE PATH "Path to pre-built DNG library and headers")

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

set(PYDNG_PY_PKG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/dngpy)
get_filename_component(PYDNG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PyDNGBindings.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PyDNGSources.cmake)

# XMP headers: Adobe code uses #if _MSC_VER <= 1600; undefined _MSC_VER is treated as 0 → broken std::tr1 on MinGW.
function(dngpy_windows_gnu_xmp_fix target)
    if(WIN32 AND NOT MSVC)
        target_compile_definitions(${target} PRIVATE _MSC_VER=1920)
    endif()
endfunction()

if(BUILD_DNG_LIBRARY)
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
    target_include_directories(dng PRIVATE ${LIBJPEG_DIR})

    if(NOT WIN32)
        ProcessorCount(N)
        ExternalProject_Add(
            libjpeg
            SOURCE_DIR ${LIBJPEG_DIR}
            BINARY_DIR ${LIBJPEG_DIR}
            CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env "CFLAGS=-fPIC -O2" "CXXFLAGS=-fPIC -O2" sh ${LIBJPEG_DIR}/configure --prefix=${CMAKE_INSTALL_PREFIX}
            BUILD_COMMAND make -j${N}
            BUILD_BYPRODUCTS ${LIBJPEG_DIR}/.libs/libjpeg.a
            INSTALL_COMMAND ""
        )
    endif()

    set(LIBJXL_EXTRA_ARGS "")
    if(UNIX)
        # -C preload runs before libjxl project(); fixes brotli log2() check on manylinux/glibc.
        list(APPEND LIBJXL_EXTRA_ARGS
            "-C${CMAKE_CURRENT_SOURCE_DIR}/cmake/LibjxlCMakeCacheInit.cmake"
            -DCMAKE_EXE_LINKER_FLAGS=-lm
            -DCMAKE_SHARED_LINKER_FLAGS=-lm
            -DCMAKE_REQUIRED_LIBRARIES=m
            -DThreads_FOUND=TRUE
            -DCMAKE_THREAD_LIBS_INIT=-pthread
            -DATOMICS_LOCK_FREE_INSTRUCTIONS=1
            -DLOG2_LIBM_RES=1
        )
    endif()

    ExternalProject_Add(
        libjxl
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjxl/libjxl
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            ${LIBJXL_EXTRA_ARGS}
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
        target_compile_options(dng PUBLIC $<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)
        target_link_libraries(dng PRIVATE "-framework CoreServices")
    elseif(WIN32)
        target_compile_definitions(dng PUBLIC qWinOS=1 WIN_ENV NOMINMAX UNICODE)
    else()
        target_compile_options(dng PUBLIC -fPIC -Wall -Wextra $<$<COMPILE_LANGUAGE:C>:-Wno-implicit-function-declaration>)
        target_compile_definitions(dng PUBLIC qLinux=1 UNIX_ENV XMP_64=1)
    endif()

    dngpy_windows_gnu_xmp_fix(dng)

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
            ${LIBJPEG_DIR}/.libs/libjpeg.a
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
else()
    if(NOT IS_ABSOLUTE "${PREBUILT_DNG_PATH}")
        set(PREBUILT_DNG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${PREBUILT_DNG_PATH}")
    endif()
    # Use pre-built library
    find_library(DNG_LIB dng PATHS ${PREBUILT_DNG_PATH}/lib ${PREBUILT_DNG_PATH}/lib64 ${PREBUILT_DNG_PATH}/dngpy NO_DEFAULT_PATH)
    if(NOT DNG_LIB)
        message(FATAL_ERROR "Pre-built dng library not found in ${PREBUILT_DNG_PATH}")
    endif()
    add_library(dng SHARED IMPORTED GLOBAL)
    set_target_properties(dng PROPERTIES IMPORTED_LOCATION ${DNG_LIB})
    if(WIN32)
        # Import library (.lib) is also needed for linking on Windows
        find_library(DNG_IMPORT_LIB dng PATHS ${PREBUILT_DNG_PATH}/lib NO_DEFAULT_PATH)
        if(DNG_IMPORT_LIB)
            set_target_properties(dng PROPERTIES IMPORTED_IMPLIB ${DNG_IMPORT_LIB})
        endif()
    endif()
    # Headers are expected in ${PREBUILT_DNG_PATH}/include/dng
    target_include_directories(dng INTERFACE ${PREBUILT_DNG_PATH}/include)
endif()

if(BUILD_DNG_VALIDATE)
    set(DNG_VALIDATE_EXTRA
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_globals.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_xmp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_host.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_xmp_sdk.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_memory.cpp
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
    dngpy_windows_gnu_xmp_fix(dng_validate)
    target_include_directories(dng_validate PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/bindings/include
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

if(BUILD_DNG_LIBRARY)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/
            DESTINATION include/dng FILES_MATCHING PATTERN "*.h")
endif()

if(WIN32)
    if(BUILD_PYTHON_BINDINGS)
        if(BUILD_DNG_LIBRARY)
            install(TARGETS dng
                RUNTIME DESTINATION dngpy
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
        else()
            install(FILES ${PREBUILT_DNG_PATH}/bin/dng.dll DESTINATION dngpy)
        endif()
    else()
        if(BUILD_DNG_LIBRARY)
            install(TARGETS dng
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
                ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
        endif()
    endif()
else()
    if(BUILD_DNG_LIBRARY)
        install(TARGETS dng LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
    else()
        file(GLOB DNG_LIBS "${PREBUILT_DNG_PATH}/lib/libdng.so*" "${PREBUILT_DNG_PATH}/lib64/libdng.so*")
        install(FILES ${DNG_LIBS} DESTINATION lib)
    endif()
endif()
if(BUILD_DNG_VALIDATE)
    install(TARGETS dng_validate DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(BUILD_PYTHON_BINDINGS)
    find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)
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
    # Wheel 将 libdng 装在 site-packages/lib/、扩展在 site-packages/dngpy/；若 RUNPATH 为构建目录的绝对路径，
    # 安装后该路径不存在会导致 ImportError。使用相对 RUNPATH 指向与 dngpy 同级的 lib/。
    if(APPLE)
        set_target_properties(_native PROPERTIES
            INSTALL_RPATH "@loader_path/../lib"
            BUILD_RPATH "${CMAKE_BINARY_DIR}")
    elseif(UNIX)
        set_target_properties(_native PROPERTIES
            INSTALL_RPATH "\$ORIGIN/../lib"
            BUILD_RPATH "${CMAKE_BINARY_DIR}")
    endif()

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
    if(APPLE)
        target_compile_definitions(_native PRIVATE qMacOS=1 MAC_ENV XMP_64=1)
    elseif(WIN32)
        target_compile_definitions(_native PRIVATE qWinOS=1 WIN_ENV NOMINMAX UNICODE)
    else()
        target_compile_definitions(_native PRIVATE qLinux=1 UNIX_ENV XMP_64=1)
    endif()
    dngpy_windows_gnu_xmp_fix(_native)

    target_link_libraries(_native PRIVATE dng)
    target_link_directories(_native PRIVATE ${CMAKE_INSTALL_PREFIX}/lib ${JXL_BUILD_DIR}/lib)

    install(TARGETS _native
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/dngpy
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/dngpy
    )
    install(FILES
        ${PYDNG_PY_PKG_DIR}/__init__.py
        ${PYDNG_PY_PKG_DIR}/__init__.pyi
        ${PYDNG_PY_PKG_DIR}/_native.pyi
        ${PYDNG_PY_PKG_DIR}/py.typed
        DESTINATION ${CMAKE_INSTALL_PREFIX}/dngpy
    )

    if(WIN32)
        add_custom_command(TARGET _native POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/python/dngpy
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${PYDNG_PY_PKG_DIR} ${CMAKE_BINARY_DIR}/python/dngpy
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:dng> ${CMAKE_BINARY_DIR}/python/dngpy/
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:_native> ${CMAKE_BINARY_DIR}/python/dngpy/
        )
    else()
        # 与 wheel 一致：libdng 在 lib/、包在 dngpy/，配合 INSTALL_RPATH $ORIGIN/../lib
        # 使用 cmake -P 复制 libdng.so*（含 symlink），避免 Ninja+sh 下 bash/shopt/cp 解析失败
        execute_process(
            COMMAND ${Python3_EXECUTABLE} -c "import site; print(site.getusersitepackages())"
            OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        set(_copy_libdng_script ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyLibDngLibs.cmake)
        add_custom_command(TARGET _native POST_BUILD
            COMMAND ${CMAKE_COMMAND} -DBIN_DIR=$<TARGET_FILE_DIR:dng> -DDEST=${CMAKE_BINARY_DIR}/python/lib
                -P ${_copy_libdng_script}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/python/dngpy
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${PYDNG_PY_PKG_DIR} ${CMAKE_BINARY_DIR}/python/dngpy
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:_native> ${CMAKE_BINARY_DIR}/python/dngpy/
            COMMENT "Staging dngpy (build/python/{lib,dngpy}) for local PYTHONPATH"
        )
        if(PYTHON_SITE_PACKAGES)
            add_custom_command(TARGET _native POST_BUILD
                COMMAND ${CMAKE_COMMAND} -DBIN_DIR=$<TARGET_FILE_DIR:dng> -DDEST=${PYTHON_SITE_PACKAGES}/lib
                    -P ${_copy_libdng_script}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_SITE_PACKAGES}/dngpy
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${PYDNG_PY_PKG_DIR} ${PYTHON_SITE_PACKAGES}/dngpy
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:_native> ${PYTHON_SITE_PACKAGES}/dngpy/
            )
        endif()
    endif()
endif()
