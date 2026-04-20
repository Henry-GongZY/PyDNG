# Preloaded with: cmake -C LibjxlCMakeCacheInit.cmake ...
# Brotli's CHECK_FUNCTION_EXISTS(log2) runs before its own ";m" fallback is reliable
# when CMAKE_REQUIRED_LIBRARIES is empty (manylinux / devtoolset).
set(CMAKE_REQUIRED_LIBRARIES "m" CACHE STRING "Libraries for try_compile (libm for log2)" FORCE)
