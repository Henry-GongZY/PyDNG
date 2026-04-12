# Source lists for libdng (included from project root CMakeLists.txt)
get_filename_component(PYDNG_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(_X ${PYDNG_ROOT}/extern/xmp/toolkit)
set(_D ${PYDNG_ROOT}/extern/dng_sdk/source)
set(_J ${PYDNG_ROOT}/extern/libjpeg)
set(ZLIB_DIR ${_X}/third-party/zlib)

set(ZLIB_SRCS
    adler32.c compress.c crc32.c deflate.c gzclose.c gzlib.c gzread.c gzwrite.c
    infback.c inffast.c inflate.c inftrees.c trees.c uncompr.c zutil.c
)
list(TRANSFORM ZLIB_SRCS PREPEND "${ZLIB_DIR}/")

set(LIBJPEG_WIN_SRCS
    jaricom.c jcapimin.c jcapistd.c jcarith.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c jcinit.c
    jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c jcprepct.c jcsample.c jctrans.c
    jdapimin.c jdapistd.c jdarith.c jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c jddctmgr.c
    jdhuff.c jdinput.c jdmainct.c jdmarker.c jdmaster.c jdmerge.c jdpostct.c jdsample.c jdtrans.c
    jerror.c jfdctflt.c jfdctfst.c jfdctint.c jidctflt.c jidctfst.c jidctint.c jmemmgr.c jmemnobs.c
    jquant1.c jquant2.c jutils.c
)
list(TRANSFORM LIBJPEG_WIN_SRCS PREPEND "${_J}/")

set(DNG_VALIDATE_SRC
    ${PYDNG_ROOT}/bindings/main.cpp
    ${PYDNG_ROOT}/bindings/dng.h
    ${PYDNG_ROOT}/bindings/dng.cpp
    ${PYDNG_ROOT}/bindings/utils.h
    ${PYDNG_ROOT}/bindings/pch.h
)

file(
    GLOB SRC_FILES
    ${_X}/source/XMP_ProgressTracker.cpp
    ${_X}/source/UnicodeConversions.cpp
    ${_X}/source/PerfUtils.cpp
    ${_X}/source/XML_Node.cpp
    ${_X}/source/XIO.cpp
    ${_X}/source/IOUtils.cpp
    ${_X}/source/XMPFiles_IO.cpp
    ${_X}/source/SafeStringAPIs.cpp
    ${_X}/source/XMP_LibUtils.cpp
    ${_X}/public/include/XMPCommon/source/*.cpp
    ${_X}/public/include/XMPCore/source/*.cpp
    ${_X}/third-party/zuid/sources/*.cpp
    ${_X}/XMPCommon/source/*.cpp
    ${_X}/XMPCore/source/DOMSerializerImpl.cpp
    ${_X}/XMPCore/source/IPathSegment_I.cpp
    ${_X}/XMPCore/source/INode_I.cpp
    ${_X}/XMPCore/source/WXMPMeta.cpp
    ${_X}/XMPCore/source/ClientDOMSerializerWrapperImpl.cpp
    ${_X}/XMPCore/source/XMPIterator2.cpp
    ${_X}/XMPCore/source/ICoreObjectFactory_I.cpp
    ${_X}/XMPCore/source/ArrayNodeImpl.cpp
    ${_X}/XMPCore/source/NameSpacePrefixMapImpl.cpp
    ${_X}/XMPCore/source/INameSpacePrefixMap_I.cpp
    ${_X}/XMPCore/source/IArrayNode_I.cpp
    ${_X}/XMPCore/source/WXMPDocOps.cpp
    ${_X}/XMPCore/source/CoreConfigurationManagerImpl.cpp
    ${_X}/XMPCore/source/XMPDocOps.cpp
    ${_X}/XMPCore/source/INodeIterator_I.cpp
    ${_X}/XMPCore/source/MetadataConverterUtilsImpl.cpp
    ${_X}/XMPCore/source/ICompositeNode_I.cpp
    ${_X}/XMPCore/source/XMPCore_Impl.cpp
    ${_X}/XMPCore/source/PathImpl.cpp
    ${_X}/XMPCore/source/ParseRDF.cpp
    ${_X}/XMPCore/source/ClientDOMParserWrapperImpl.cpp
    ${_X}/XMPCore/source/StructureNodeImpl.cpp
    ${_X}/XMPCore/source/ICoreConfigurationManager_I.cpp
    ${_X}/XMPCore/source/XMPUtils.cpp
    ${_X}/XMPCore/source/ISimpleNode_I.cpp
    ${_X}/XMPCore/source/IDOMParser_I.cpp
    ${_X}/XMPCore/source/XMPIterator.cpp
    ${_X}/XMPCore/source/IStructureNode_I.cpp
    ${_X}/XMPCore/source/XMPUtils2.cpp
    ${_X}/XMPCore/source/WXMPIterator.cpp
    ${_X}/XMPCore/source/IDOMImplementationRegistry_I.cpp
    ${_X}/XMPCore/source/IPath_I.cpp
    ${_X}/XMPCore/source/MetadataImpl.cpp
    ${_X}/XMPCore/source/XMPDocOps2.cpp
    ${_X}/XMPCore/source/IDOMSerializer_I.cpp
    ${_X}/XMPCore/source/PathSegmentImpl.cpp
    ${_X}/XMPCore/source/RDFDOMParserImpl.cpp
    ${_X}/XMPCore/source/ExpatAdapter.cpp
    ${_X}/XMPCore/source/DOMImplementationRegistryImpl.cpp
    ${_X}/XMPCore/source/IMetadata_I.cpp
    ${_X}/XMPCore/source/RDFDOMSerializerImpl.cpp
    ${_X}/XMPCore/source/DOMParserImpl.cpp
    ${_X}/XMPCore/source/CompositeNodeImpl.cpp
    ${_X}/XMPCore/source/XMPMeta.cpp
    ${_X}/XMPCore/source/CoreObjectFactoryImpl.cpp
    ${_X}/XMPCore/source/SimpleNodeImpl.cpp
    ${_X}/XMPCore/source/WXMPUtils.cpp
    ${_X}/XMPCore/source/NodeImpl.cpp
    ${_X}/XMPCore/source/IMetadataConverterUtils_I.cpp
    ${_X}/XMPCore/source/XMPMeta-Parse.cpp
    ${_X}/XMPCore/source/XMPDocOps-Utils.cpp
    ${_X}/XMPCore/source/XMPMeta-Serialize.cpp
    ${_X}/XMPCore/source/XMPMeta-GetSet.cpp
    ${_X}/XMPCore/source/XMPMeta2-GetSet.cpp
    ${_X}/XMPCore/source/XMPUtils-FileInfo.cpp
    ${_X}/XMPCore/third-party/expat/public/lib/*.c
    ${_X}/XMPFiles/source/*.cpp
    ${_X}/XMPFiles/source/FileHandlers/*.cpp
    ${_X}/XMPFiles/source/FormatSupport/*.cpp
    ${_X}/XMPFiles/source/FormatSupport/AIFF/*.cpp
    ${_X}/XMPFiles/source/FormatSupport/IFF/*.cpp
    ${_X}/XMPFiles/source/FormatSupport/WAVE/*.cpp
    ${_X}/XMPFiles/source/FormatSupport/WebP/*.cpp
    ${_X}/XMPFiles/source/NativeMetadataSupport/*.cpp
    ${_X}/XMPFiles/source/PluginHandler/FileHandlerInstance.cpp
    ${_X}/XMPFiles/source/PluginHandler/HostAPIImpl.cpp
    ${_X}/XMPFiles/source/PluginHandler/Module.cpp
    ${_X}/XMPFiles/source/PluginHandler/PluginManager.cpp
    ${_X}/XMPFiles/source/PluginHandler/XMPAtoms.cpp
    ${_X}/XMPFiles/source/WXMPFiles.cpp
    ${_X}/public/include/client-glue/*.incl_cpp
    ${_D}/*.cpp
)

if(APPLE)
    list(APPEND SRC_FILES
        ${_X}/XMPFiles/source/PluginHandler/OS_Utils_Mac.cpp
        ${_X}/source/Host_IO-POSIX.cpp
        ${ZLIB_SRCS}
    )
elseif(WIN32)
    list(APPEND SRC_FILES
        ${_X}/XMPFiles/source/PluginHandler/OS_Utils_WIN.cpp
        ${_X}/source/Host_IO-Win.cpp
        ${ZLIB_SRCS}
        ${LIBJPEG_WIN_SRCS}
    )
else()
    list(APPEND SRC_FILES
        ${_X}/XMPFiles/source/PluginHandler/OS_Utils_Linux.cpp
        ${_X}/source/Host_IO-POSIX.cpp
        ${ZLIB_SRCS}
    )
endif()

# Variables visible in directory scope after include()
