"""
PyDNG: Python bindings for Adobe DNG SDK

This module provides Python bindings for reading and writing DNG (Digital Negative) files.
"""

import sys
import os

# Add build directories to path for development
_build_dirs = [
    os.path.join(os.path.dirname(__file__), '..', 'build', 'python'),
    os.path.join(os.path.dirname(__file__), '..', 'build', 'Release'),
]

for build_dir in _build_dirs:
    abs_build_dir = os.path.abspath(build_dir)
    if os.path.exists(abs_build_dir) and abs_build_dir not in sys.path:
        sys.path.insert(0, abs_build_dir)

# Import the compiled module
# Clear any existing import to avoid conflicts
if 'pydng' in sys.modules:
    del sys.modules['pydng']

try:
    import pydng as _pydng_core
except ImportError as e:
    raise ImportError(
        f"Failed to import pydng module: {e}\n"
        "Please ensure:\n"
        "1. The module is built (run CMake build)\n"
        "2. The .pyd/.so file is in build/python or installed in site-packages\n"
        "3. All dependencies (dng.dll/libdng.so) are available"
    )

# Export only classes, no constants
Dng = _pydng_core.Dng
DngMeta = _pydng_core.DngMeta
DngData = _pydng_core.DngData
ErrorCode = _pydng_core.ErrorCode

__version__ = "0.1.0"
__all__ = [
    "Dng",
    "DngMeta",
    "DngData",
    "ErrorCode",
]
