"""
Type stubs for PyDNG Python bindings
"""

from typing import Any, overload, Union
from numpy.typing import NDArray

__version__: str

# Error code type
ErrorCodeType = int

class ErrorCode:
    """Error code constants for DNG operations"""
    NONE: ErrorCodeType
    READ_FILE: ErrorCodeType
    WRITE_FILE: ErrorCodeType
    BAD_FORMAT: ErrorCodeType
    UNKNOWN: ErrorCodeType

class DngMeta:
    """Metadata information for a DNG file"""
    
    # Basic information
    make: str
    model: str
    software: str
    artist: str
    copyright: str
    
    # Image dimensions
    width: int
    height: int
    raw_width: int
    raw_height: int
    
    # Camera settings
    exposure_time: float
    f_number: float
    focal_length: float
    iso: int
    focal_length_35mm: int
    
    # Date/time
    date_time: str
    date_time_original: str
    
    # Other information
    is_monochrome: bool
    color_planes: int
    color_space: str
    
    def __init__(self) -> None: ...
    def __repr__(self) -> str: ...

class DngGainMap:
    """Gain map information for a DNG file (typically used for shading correction)"""
    
    rows: int
    cols: int
    planes: int
    spacing_v: float
    spacing_h: float
    origin_v: float
    origin_h: float
    data: NDArray[Any]
    
    def __init__(self) -> None: ...
    
    def to_numpy(self) -> NDArray[Any]:
        """
        Convert GainMap data to numpy array.
        
        Returns:
            numpy.ndarray: Gain map data as float32 array with shape (rows, cols, planes).
        """
        ...

class DngData:
    """Image dimensions and layout returned by :meth:`Dng.get_data_info`."""
    
    width: int  # Read-only
    height: int  # Read-only
    channels: int  # Read-only
    pixel_type: int  # Read-only
    top: int  # Read-only
    left: int  # Read-only
    
    def __init__(self) -> None: ...
    
    def __repr__(self) -> str: ...

class Dng:
    """Read an existing DNG file and expose a NumPy-first API."""

    def __init__(self, path: str, ignore_enhanced: bool = False) -> None:
        """Load a DNG from ``path``. Raises ``RuntimeError`` if reading fails."""
        ...

    @property
    def raw_pixels(self) -> NDArray[Any]:
        """Stage 1 RAW pixels as an independent NumPy array."""
        ...

    @property
    def enhanced_pixels(self) -> NDArray[Any]:
        """Stage 3 pixels as an independent NumPy array."""
        ...

    @property
    def metadata(self) -> DngMeta:
        """A value snapshot of the DNG metadata."""
        ...

    @property
    def image_info(self) -> DngMeta:
        """A value snapshot of image dimensions and crop geometry."""
        ...

    @property
    def exif(self) -> DngMeta:
        """A value snapshot of the DNG EXIF metadata."""
        ...

    @property
    def color_info(self) -> DngMeta:
        """A value snapshot of color planes and color space."""
        ...

    def save(self, path: str) -> int:
        """Write this DNG to ``path``; raise ``RuntimeError`` on failure."""
        ...

    def set_raw_pixels(self, pixels: NDArray[Any], enhanced: bool = False) -> None:
        """Replace Stage 1 RAW pixels or, with ``enhanced=True``, Stage 3 pixels."""
        ...
    
    def get_data_info(self, enhanced: bool = False) -> DngData:
        """Get image dimensions, pixel type, and active-area offset without copying pixels."""
        ...
    
    def get_baseline_exposure(self) -> float:
        """
        Get baseline exposure value.
        
        Returns:
            float: Baseline exposure in EV (exposure value)
        """
        ...
    
    def set_baseline_exposure(self, exposure: float) -> None:
        """
        Set baseline exposure value.
        
        Args:
            exposure: Baseline exposure in EV (exposure value)
        """
        ...
    
    def set_meta(self, meta: DngMeta) -> None:
        """
        Set metadata for DNG file.
        
        Args:
            meta: DngMeta object containing metadata to set
        """
        ...

    def get_white_balance(self) -> list[float]:
        """
        Get white balance neutral vector (AsShotNeutral).
        
        Returns:
            list[float]: Neutral vector gains, typically [r, g, b]
        """
        ...
    
    def set_white_balance(self, wb: list[float]) -> None:
        """
        Set white balance neutral vector (AsShotNeutral).
        
        Args:
            wb: List of gains, e.g., [r, g, b]
        """
        ...

    def get_gainmap(self) -> DngGainMap:
        """
        Get gain map (shading correction) from DNG file.
        
        Returns:
            DngGainMap: Gain map object containing sampled scale factors.
        """
        ...
    
    def set_gainmap(self, map: DngGainMap) -> None:
        """
        Set gain map (shading correction) for DNG file.
        
        Args:
            map: DngGainMap object to set.
        """
        ...

    def get_bayer_pattern(self) -> str:
        """2×2 Bayer tile as ``RGGB`` / ``GRBG`` / ``BGGR`` / ``GBRG``, or ``\"\"`` if not a 2×2 RGB CFA."""
        ...

    def set_bayer_pattern(self, pattern: str) -> None:
        """
        Set the 2×2 Bayer phase. ``pattern`` must be one of
        ``RGGB``, ``GRBG``, ``BGGR``, ``GBRG`` (case-insensitive).

        Raises:
            ValueError: Invalid pattern or unsupported CFA plane count.
            RuntimeError: No image loaded.
        """
        ...

# Type aliases for pixel types (for documentation purposes)
# These are not actual constants, but type hints for clarity
PixelType: type = Union[int, str]
"""Pixel type values:
- "uint8" (1): 8-bit unsigned integer
- "uint16" (3): 16-bit unsigned integer
- "int16" (8): 16-bit signed integer
- "uint32" (4): 32-bit unsigned integer
"""

__all__ = [
    "Dng",
    "DngMeta",
    "DngData",
    "DngGainMap",
    "ErrorCode",
    "ErrorCodeType",
    "PixelType",
]

