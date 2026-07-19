"""
PyDNG: Python bindings for Adobe DNG SDK

This module provides Python bindings for reading and writing DNG (Digital Negative) files.
"""

import os
from pathlib import Path

import numpy as np

# Python 3.8+ restricts DLL search on Windows. Register the package directory so that
# dng.dll (shipped alongside _native.pyd) is found when the extension is loaded.
if hasattr(os, "add_dll_directory"):
    os.add_dll_directory(str(Path(__file__).parent))

from ._native import Dng as _NativeDng
from ._native import DngData, DngMeta, ErrorCode

__version__ = "0.1.3"

PIXEL_TYPE_MAP = {
    "uint8": 1,    # ttByte
    "uint16": 3,   # ttShort
    "int16": 8,    # ttSShort
    "uint32": 4,   # ttLong
    "float32": 11, # ttFloat
}

PIXEL_TYPE_FOR_DTYPE = {
    np.dtype(np.uint8): "uint8",
    np.dtype(np.uint16): "uint16",
    np.dtype(np.int16): "int16",
    np.dtype(np.uint32): "uint32",
    np.dtype(np.float32): "float32",
}

class Dng(_NativeDng):
    """Read and write DNG files with a NumPy-first API."""

    @property
    def raw_pixels(self):
        """Stage 1 RAW pixels as an independent NumPy array."""
        return super()._get_pixels(enhanced=False)

    @property
    def enhanced_pixels(self):
        """Stage 3 pixels as an independent NumPy array, if present."""
        return super()._get_pixels(enhanced=True)

    @property
    def metadata(self):
        """A value snapshot of the DNG metadata."""
        return super()._get_metadata()

    @property
    def image_info(self):
        """A value snapshot of image dimensions and crop geometry."""
        return super()._get_image_info()

    @property
    def exif(self):
        """A value snapshot of the DNG EXIF metadata."""
        return super()._get_exif()

    @property
    def color_info(self):
        """A value snapshot of color planes and color space."""
        return super()._get_color_info()

    def save(self, path):
        """Write this DNG to ``path``; raise ``RuntimeError`` on failure."""
        return super()._save(path)

    def set_raw_pixels(self, pixels, enhanced=False):
        """Replace Stage 1 RAW pixels or, with ``enhanced=True``, Stage 3 pixels."""
        pixels = np.asarray(pixels)
        try:
            pixel_type_name = PIXEL_TYPE_FOR_DTYPE[pixels.dtype]
        except KeyError as exc:
            raise ValueError(
                f"Cannot infer a DNG pixel type from dtype {pixels.dtype}; "
                f"use one of {list(PIXEL_TYPE_MAP)}"
            ) from exc
        pixel_type = PIXEL_TYPE_MAP[pixel_type_name]
        return super()._set_pixels(pixels, pixel_type, enhanced=enhanced)

    def get_data_info(self, enhanced=False):
        """Return image dimensions, pixel type, and active-area offset."""
        return super()._get_data_info(enhanced=enhanced)

__all__ = [
    "Dng",
    "DngMeta",
    "DngData",
    "ErrorCode",
    "PIXEL_TYPE_FOR_DTYPE",
]
