# Building DNG SDK
cmake_minimum_required(VERSION 3.15)

# default to release build except Windows
if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "build type")
endif()
set(BUILD_DNG_VALIDATE OFF CACHE BOOL "Build dng_validate command")

# Define project name and (optional) version
project(dng VERSION 1.7.1 DESCRIPTION "DNG SDK dynamic library" LANGUAGES C CXX)

# 设置默认安装路径为本地目录，避免需要管理员权限
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation Directory" FORCE)
endif()
message(STATUS "Installation directory: ${CMAKE_INSTALL_PREFIX}")

# need >= C++11
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# CMake functionality
include(GNUInstallDirs)
include(ExternalProject)
include(ProcessorCount)

# configs
if(WIN32)
    # Windows: FIXME
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# List source files for the library
set(DNG_VALIDATE_SRC "pydng/main.cpp"
    ${CMAKE_CURRENT_SOURCE_DIR}/pydng/dng.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pydng/dng.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pydng/utils.h
        pydng/pch.h)

file(
    GLOB SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/source/XMP_ProgressTracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/source/UnicodeConversions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/source/PerfUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/source/XML_Node.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/source/XIO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/source/IOUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/source/XMPFiles_IO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/source/SafeStringAPIs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/source/XMP_LibUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/public/include/XMPCommon/source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/public/include/XMPCore/source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zuid/sources/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCommon/source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/DOMSerializerImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/IPathSegment_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/INode_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/WXMPMeta.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/ClientDOMSerializerWrapperImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/XMPIterator2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/ICoreObjectFactory_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/ArrayNodeImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/NameSpacePrefixMapImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/INameSpacePrefixMap_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/IArrayNode_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/WXMPDocOps.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/CoreConfigurationManagerImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/XMPDocOps.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/INodeIterator_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/MetadataConverterUtilsImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/ICompositeNode_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/XMPCore_Impl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/PathImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/ParseRDF.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/ClientDOMParserWrapperImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/StructureNodeImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/ICoreConfigurationManager_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/XMPUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/ISimpleNode_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/IDOMParser_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/XMPIterator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/IStructureNode_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/XMPUtils2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/WXMPIterator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/IDOMImplementationRegistry_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/IPath_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/MetadataImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/XMPDocOps2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/IDOMSerializer_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/PathSegmentImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/RDFDOMParserImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/ExpatAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/DOMImplementationRegistryImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/IMetadata_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/RDFDOMSerializerImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/DOMParserImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/CompositeNodeImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/XMPMeta.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/CoreObjectFactoryImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/SimpleNodeImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/WXMPUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/NodeImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/IMetadataConverterUtils_I.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/XMPMeta-Parse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/XMPDocOps-Utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/XMPMeta-Serialize.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/XMPMeta-GetSet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/XMPMeta2-GetSet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/source/XMPUtils-FileInfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/third-party/expat/public/lib/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/FileHandlers/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/FormatSupport/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/FormatSupport/AIFF/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/FormatSupport/IFF/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/FormatSupport/WAVE/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/FormatSupport/WebP/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/NativeMetadataSupport/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/PluginHandler/FileHandlerInstance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/PluginHandler/HostAPIImpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/PluginHandler/Module.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/PluginHandler/PluginManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/PluginHandler/XMPAtoms.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/WXMPFiles.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/public/include/client-glue/*.incl_cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/*.cpp
)

# add platform-specific source files
if(APPLE)
    list(
        APPEND SRC_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/PluginHandler/OS_Utils_Mac.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/source/Host_IO-POSIX.cpp
        # zlib - add source files directly
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/adler32.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/compress.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/crc32.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/deflate.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/gzclose.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/gzlib.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/gzread.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/gzwrite.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/infback.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/inffast.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/inflate.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/inftrees.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/trees.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/uncompr.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/zutil.c
    )
elseif(WIN32)
    list(
        APPEND SRC_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/PluginHandler/OS_Utils_WIN.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/source/Host_IO-Win.cpp
        # zlib - add source files directly for Windows
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/adler32.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/compress.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/crc32.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/deflate.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/gzclose.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/gzlib.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/gzread.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/gzwrite.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/infback.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/inffast.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/inflate.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/inftrees.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/trees.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/uncompr.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/zutil.c
        # libjpeg - add source files directly for Windows
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jaricom.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jcapimin.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jcapistd.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jcarith.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jccoefct.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jccolor.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jcdctmgr.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jchuff.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jcinit.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jcmainct.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jcmarker.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jcmaster.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jcomapi.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jcparam.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jcprepct.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jcsample.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jctrans.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdapimin.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdapistd.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdarith.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdatadst.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdatasrc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdcoefct.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdcolor.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jddctmgr.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdhuff.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdinput.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdmainct.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdmarker.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdmaster.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdmerge.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdpostct.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdsample.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jdtrans.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jerror.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jfdctflt.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jfdctfst.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jfdctint.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jidctflt.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jidctfst.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jidctint.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jmemmgr.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jmemnobs.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jquant1.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jquant2.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/jutils.c
    )
else()  # UNIX/Linux
    list(
        APPEND SRC_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFiles/source/PluginHandler/OS_Utils_Linux.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/source/Host_IO-POSIX.cpp
        # zlib - add source files directly
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/adler32.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/compress.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/crc32.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/deflate.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/gzclose.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/gzlib.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/gzread.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/gzwrite.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/infback.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/inffast.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/inflate.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/inftrees.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/trees.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/uncompr.c
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib/zutil.c
    )
endif()

# Create the shared library
add_library(dng SHARED ${SRC_FILES})

# set library version information
set_target_properties(
    dng PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# ----- dependencies
# zlib (using local path)
set(ZLIB_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/third-party/zlib)
set(ZLIB_INCLUDE_DIRS ${ZLIB_ROOT})
target_include_directories(dng PRIVATE ${ZLIB_ROOT})
message(STATUS "Using local zlib from: ${ZLIB_ROOT}")

# libjpeg
if(WIN32)
    target_include_directories(dng PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg)
else()
    ProcessorCount(N) # for parallel execution
    ExternalProject_Add(
        libjpeg
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg
        BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg
        CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjpeg/configure --prefix=${CMAKE_INSTALL_PREFIX}
        BUILD_COMMAND make -j${N}
    )
endif()

# libjxl - 配置为静态链接以避免依赖系统库
ExternalProject_Add(
    libjxl
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/libjxl/libjxl
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
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

# common setting
if(NOT WIN32)
    add_dependencies(dng libjpeg)
endif()
add_dependencies(dng libjxl)
target_compile_definitions(
    dng PRIVATE
    $<IF:$<CONFIG:Debug>,qDNGDebug=1,qDNGDebug=0>
    qDNGValidate=1 qDNGUseXMP=1 qDNGUseLibJPEG=1 AdobePrivate=1
    BUILDING_XMPCORE_LIB=1 BUILDING_XMPCORE_AS_STATIC=1 XMP_StaticBuild=1 XMP_COMPONENT_INT_NAMESPACE=AdobeXMPCore_Int
    JXL_STATIC_DEFINE=1  # 告诉 JXL 使用静态链接
)

if(${BUILD_DNG_VALIDATE})
    target_compile_definitions(dng PUBLIC qDNGValidateTarget=1)
else()
    target_compile_definitions(dng PUBLIC qDNGValidateTarget=0)
endif()

# platform-specific donfigs
if(APPLE)
    target_compile_definitions(dng PUBLIC qMacOS=1 MAC_ENV XMP_64=1)
    target_compile_options(dng PUBLIC -Wno-implicit-function-declaration)
    target_link_libraries(dng PRIVATE "-framework CoreServices")
elseif(WIN32)
    target_compile_definitions(dng PUBLIC qWinOS=1 WIN_ENV NOMINMAX UNICODE)
else()  # UNIX/Linux
    target_compile_options(dng PUBLIC -fPIC -Wall -Wextra -Wno-implicit-function-declaration)
    target_compile_definitions(dng PUBLIC qLinux=1 UNIX_ENV XMP_64=1)
endif()

# Set include directory for headers
target_include_directories(
    dng PRIVATE
    ${ZLIB_INCLUDE_DIRS}
    ${CMAKE_INSTALL_PREFIX}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/public/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPFilesPlugins/api/source
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/third-party/expat/public/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/XMPCore/third-party/boost
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/public/include/XMPCore/Interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/xmp/toolkit/public/include/client-glue
)

# 添加链接目录：包括 install 目录和 libjxl 构建目录
target_link_directories(dng PRIVATE
    ${CMAKE_INSTALL_PREFIX}/lib
    ${CMAKE_BINARY_DIR}/libjxl-prefix/src/libjxl-build/lib
    ${CMAKE_BINARY_DIR}/libjxl-prefix/src/libjxl-build/third_party/brotli
    ${CMAKE_BINARY_DIR}/libjxl-prefix/src/libjxl-build/third_party/highway
)

# 链接库配置
if(WIN32)
    # Windows: 链接 JXL 静态库及其所有依赖
    # zlib和libjpeg源码已经直接编译到dng库中
    target_link_libraries(dng PRIVATE
        jxl-static          # JXL 完整库（包含编码器和解码器）
        jxl_dec-static      # JXL 解码器
        jxl_threads-static  # JXL 线程支持
        hwy                 # Highway SIMD 库
        brotlienc-static    # Brotli 编码器
        brotlidec-static    # Brotli 解码器
        brotlicommon-static # Brotli 公共部分
    )
else()
    # Linux/macOS: 链接 JPEG 和 JXL
    # Linux: libjxl 静态库产物为 libjxl.a / libjxl_threads.a（OUTPUT_NAME jxl / jxl_threads）
    # macOS: 保持 jxl-static / jxl_threads-static（已验证可编译）
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # 与 Windows 一致：libjxl 静态库依赖 Highway / Brotli，链进共享库时必须显式链入，否则会出现未定义符号（如 BrotliDecoderErrorString）
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
        target_link_libraries(dng PRIVATE
            jpeg
            jxl-static
            jxl_threads-static
        )
    endif()
    # zlib源码已经直接编译到dng库中，无需额外链接
endif()

if(BUILD_DNG_VALIDATE)
    message("build dng_validate...")

    # 找到 DNG SDK 所需的全部源文件
    file(GLOB DNG_VALIDATE_EXTRA_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_globals.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_xmp.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_host.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_xmp_sdk.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_memory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_sdk.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/dng_bottlenecks.cpp
    )

    add_executable(dng_validate
        ${DNG_VALIDATE_SRC}
        ${DNG_VALIDATE_EXTRA_SRC}
        pydng/pch.h
    )

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


# install header & library files (& dng_validate command)
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/extern/dng_sdk/source/
    DESTINATION include/dng
    FILES_MATCHING PATTERN "*.h"
)
install(TARGETS dng LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
if(BUILD_DNG_VALIDATE)
    install(TARGETS dng_validate DESTINATION ${CMAKE_INSTALL_BINDIR})
endif(BUILD_DNG_VALIDATE)

# Python bindings with pybind11
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)

if(BUILD_PYTHON_BINDINGS)
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
    
    # Use pybind11 from extern directory (git submodule)
    set(PYBIND11_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern/pybind11")
    
    # Check if pybind11 submodule exists
    if(NOT EXISTS "${PYBIND11_DIR}/CMakeLists.txt")
        message(WARNING 
            "pybind11 not found in extern/pybind11.\n"
            "Please initialize the git submodule:\n"
            "  git submodule update --init --recursive\n"
            "Or add it manually:\n"
            "  git submodule add -b v2.11.1 https://github.com/pybind/pybind11.git extern/pybind11\n"
            "  git submodule update --init --recursive\n"
            "\n"
            "Falling back to FetchContent..."
        )
        # Fallback to FetchContent if submodule not available
        find_package(pybind11 QUIET)
        if(NOT pybind11_FOUND)
            include(FetchContent)
            FetchContent_Declare(
                pybind11
                GIT_REPOSITORY https://github.com/pybind/pybind11.git
                GIT_TAG v2.11.1
            )
            FetchContent_MakeAvailable(pybind11)
        endif()
    else()
        # Add pybind11 as a subdirectory
        add_subdirectory(${PYBIND11_DIR} ${CMAKE_BINARY_DIR}/pybind11)
        message(STATUS "Using pybind11 from submodule: ${PYBIND11_DIR}")
    endif()
    
    message(STATUS "Python executable: ${Python3_EXECUTABLE}")
    message(STATUS "Python version: ${Python3_VERSION}")
    message(STATUS "Python include dirs: ${Python3_INCLUDE_DIRS}")
    message(STATUS "Python libraries: ${Python3_LIBRARIES}")
    
    # Python extension module
    pybind11_add_module(pydng
        pydng/pydng_bindings.cpp
        pydng/dng.cpp
    )
    
    # Set target properties
    set_target_properties(pydng PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON
    )
    
    # Include directories
    target_include_directories(pydng PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/pydng
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
    
    # Compile definitions
    target_compile_definitions(pydng PRIVATE
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
        target_compile_definitions(pydng PRIVATE qWinOS=1 WIN_ENV NOMINMAX UNICODE)
    endif()
    
    # Link libraries
    target_link_libraries(pydng PRIVATE
        dng
        ${Python3_LIBRARIES}
    )
    
    # Link directories
    target_link_directories(pydng PRIVATE
        ${CMAKE_INSTALL_PREFIX}/lib
        ${CMAKE_BINARY_DIR}/libjxl-prefix/src/libjxl-build/lib
    )
    
    # Install Python module
    # scikit-build-core will automatically detect and install Python packages
    # Install the compiled module
    install(TARGETS pydng
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/pydng
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/pydng
    )
    
    # Install Python package files
    install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/pydng/__init__.py
        ${CMAKE_CURRENT_SOURCE_DIR}/pydng/__init__.pyi
        ${CMAKE_CURRENT_SOURCE_DIR}/pydng/pydng.pyi
        ${CMAKE_CURRENT_SOURCE_DIR}/pydng/py.typed
        DESTINATION ${CMAKE_INSTALL_PREFIX}/pydng
    )
    
    # Copy to Python site-packages for development
    if(WIN32)
        # On Windows, copy to a local python directory
        add_custom_command(TARGET pydng POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/python
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pydng> ${CMAKE_BINARY_DIR}/python/
            COMMENT "Copying pydng module to python directory"
        )
    else()
        # On Unix, copy to user site-packages (writable without sudo).
        # getsitepackages()[0] is often system dir (e.g. /usr/local/...) and fails for normal users.
        execute_process(
            COMMAND ${Python3_EXECUTABLE} -c "import site; print(site.getusersitepackages())"
            OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(PYTHON_SITE_PACKAGES)
            message(STATUS "Python user site-packages (POST_BUILD copy): ${PYTHON_SITE_PACKAGES}")
            add_custom_command(TARGET pydng POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory ${PYTHON_SITE_PACKAGES}
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pydng> ${PYTHON_SITE_PACKAGES}/
                COMMENT "Copying pydng module to Python user site-packages"
            )
        endif()
    endif()
endif()