"""
Type stubs for PyDNG Python bindings
"""

from typing import Any
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

class DngData:
    """Image data container for DNG files"""
    
    width: int  # Read-only
    height: int  # Read-only
    channels: int  # Read-only
    pixel_type: int  # Read-only
    top: int  # Read-only
    left: int  # Read-only
    
    def __init__(self) -> None: ...
    
    def to_numpy(self) -> NDArray[Any]:
        """
        Convert DngData to numpy array.
        
        Returns:
            numpy.ndarray: Image data as numpy array with shape (height, width, channels).
                The dtype depends on pixel_type:
                - pixel_type=1 (ttByte): uint8
                - pixel_type=3 (ttShort): uint16
                - pixel_type=8 (ttSShort): int16
                - pixel_type=4 (ttLong): uint32
        """
        ...
    
    def __repr__(self) -> str: ...

class Dng:
    """Main class for reading and writing DNG files"""
    
    def __init__(self) -> None:
        """Create a new Dng object"""
        ...
    
    def read(self, path: str, ignore_enhanced: bool = False) -> ErrorCodeType:
        """
        Read a DNG file from disk.
        
        Args:
            path: Path to the DNG file
            ignore_enhanced: If True, ignore enhanced image IFD
        
        Returns:
            ErrorCodeType: Error code (ErrorCode.NONE on success)
        """
        ...
    
    def write(self, path: str) -> ErrorCodeType:
        """
        Write the DNG file to disk.
        
        Args:
            path: Output file path
        
        Returns:
            ErrorCodeType: Error code (ErrorCode.NONE on success)
        """
        ...
    
    def get_data(self, enhanced: bool = False) -> DngData:
        """
        Get image data as DngData object.
        
        Args:
            enhanced: If True, return Stage3 (enhanced) image, 
                     otherwise return Stage1 (raw) image
        
        Returns:
            DngData: Image data object. The object manages its own memory
                    and will be freed when converted to numpy array.
        
        Raises:
            RuntimeError: If no image data is available
        """
        ...
    
    def set_data(
        self, 
        data: NDArray[Any], 
        pixel_type: int, 
        enhanced: bool = False
    ) -> None:
        """
        Set image data from numpy array.
        
        Args:
            data: NumPy array with shape (height, width, channels).
                  Supported dtypes: uint8, uint16, int16
            pixel_type: Pixel type value:
                        - 1 = ttByte (8-bit unsigned)
                        - 3 = ttShort (16-bit unsigned)
                        - 8 = ttSShort (16-bit signed)
                        - 4 = ttLong (32-bit unsigned)
            enhanced: If True, set as Stage3 (enhanced) image,
                     otherwise set as Stage1 (raw) image
        
        Raises:
            RuntimeError: If data format is invalid
        """
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
    
    def get_meta(self) -> DngMeta:
        """
        Get metadata from DNG file.
        
        Returns:
            DngMeta: Metadata object containing EXIF and other information
        """
        ...
    
    def set_meta(self, meta: DngMeta) -> None:
        """
        Set metadata for DNG file.
        
        Args:
            meta: DngMeta object containing metadata to set
        """
        ...

# Type aliases for pixel types (for documentation purposes)
# These are not actual constants, but type hints for clarity
PixelType: type = int
"""Pixel type values:
- 1: ttByte (8-bit unsigned integer)
- 3: ttShort (16-bit unsigned integer)
- 8: ttSShort (16-bit signed integer)
- 4: ttLong (32-bit unsigned integer)
"""

__all__ = [
    "Dng",
    "DngMeta",
    "DngData",
    "ErrorCode",
    "ErrorCodeType",
    "PixelType",
]

