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

from ._native import Dng as _NativeDng
from ._native import DngData, DngMeta, ErrorCode

__version__ = "0.1.0"

PIXEL_TYPE_MAP = {
    "uint8": 1,   # ttByte
    "uint16": 3,  # ttShort
    "int16": 8,   # ttSShort
    "uint32": 4,  # ttLong
}

class Dng(_NativeDng):
    """Main class for reading and writing DNG files"""
    
    def set_data(self, data, pixel_type, enhanced=False):
        """
        Set image data from numpy array.
        
        Args:
            data: NumPy array with shape (height, width, channels).
            pixel_type: Pixel type as string ("uint8", "uint16", "int16", "uint32") 
                       or integer value (1, 3, 8, 4).
            enhanced: If True, set as Stage3 (enhanced) image.
        """
        if isinstance(pixel_type, str):
            if pixel_type not in PIXEL_TYPE_MAP:
                raise ValueError(
                    f"Invalid pixel_type string: '{pixel_type}'. "
                    f"Supported: {list(PIXEL_TYPE_MAP.keys())}"
                )
            pixel_type = PIXEL_TYPE_MAP[pixel_type]
            
        return super().set_data(data, pixel_type, enhanced)

__all__ = [
    "Dng",
    "DngMeta",
    "DngData",
    "ErrorCode",
]
