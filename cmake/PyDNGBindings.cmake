# Paths and source lists for bindings/ (pybind11 extension + optional debug main).
# PYDNG_ROOT must be the project root directory (set in top-level CMakeLists.txt before include).

if(NOT DEFINED PYDNG_ROOT)
    get_filename_component(PYDNG_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()

set(PYDNG_BINDINGS_ROOT "${PYDNG_ROOT}/bindings")
set(PYDNG_BINDINGS_INCLUDE_DIR "${PYDNG_BINDINGS_ROOT}/include")

set(PYDNG_BINDINGS_SOURCES
    "${PYDNG_BINDINGS_ROOT}/src/pydng_bindings.cpp"
    "${PYDNG_BINDINGS_ROOT}/src/dng.cpp"
)

# Optional dng_validate executable (same Dng wrapper sources as the extension)
set(DNG_VALIDATE_SRC
    "${PYDNG_BINDINGS_ROOT}/main.cpp"
    "${PYDNG_BINDINGS_INCLUDE_DIR}/dng.h"
    "${PYDNG_BINDINGS_ROOT}/src/dng.cpp"
    "${PYDNG_BINDINGS_INCLUDE_DIR}/utils.h"
    "${PYDNG_BINDINGS_INCLUDE_DIR}/pch.h"
)
