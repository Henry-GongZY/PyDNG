# Copy libdng.so* from build tree into DEST (CMake preserves symlinks as symlinks when possible).
# Invoked as: cmake -DBIN_DIR=<dir> -DDEST=<dir> -P cmake/CopyLibDngLibs.cmake
if(NOT BIN_DIR OR NOT DEST)
  message(FATAL_ERROR "CopyLibDngLibs: BIN_DIR and DEST must be set")
endif()
file(MAKE_DIRECTORY "${DEST}")
file(GLOB _dng_libs LIST_DIRECTORIES false "${BIN_DIR}/libdng.so*")
if(NOT _dng_libs)
  message(WARNING "CopyLibDngLibs: no matches for ${BIN_DIR}/libdng.so*")
else()
  foreach(_f IN LISTS _dng_libs)
    file(COPY "${_f}" DESTINATION "${DEST}")
  endforeach()
endif()
