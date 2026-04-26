"""
PyDNG: Python bindings for Adobe DNG SDK

This module provides Python bindings for reading and writing DNG (Digital Negative) files.
"""

import os
from pathlib import Path

# Python 3.8+ restricts DLL search on Windows. Register the package directory so that
# dng.dll (shipped alongside _native.pyd) is found when the extension is loaded.
if hasattr(os, "add_dll_directory"):
    os.add_dll_directory(str(Path(__file__).parent))

from ._native import Dng, DngData, DngMeta, ErrorCode

__version__ = "0.1.0"
__all__ = [
    "Dng",
    "DngMeta",
    "DngData",
    "ErrorCode",
]
