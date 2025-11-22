"""
Type stubs for the compiled pydng module (internal)
This file provides type hints for the C++ extension module.
"""

from typing import Any
from numpy.typing import NDArray

class ErrorCode:
    """Error code constants"""
    NONE: int
    READ_FILE: int
    WRITE_FILE: int
    BAD_FORMAT: int
    UNKNOWN: int

class DngMeta:
    """DNG metadata"""
    make: str
    model: str
    software: str
    artist: str
    copyright: str
    width: int
    height: int
    raw_width: int
    raw_height: int
    exposure_time: float
    f_number: float
    focal_length: float
    iso: int
    focal_length_35mm: int
    date_time: str
    date_time_original: str
    is_monochrome: bool
    color_planes: int
    color_space: str
    
    def __init__(self) -> None: ...

class DngData:
    """DNG image data"""
    width: int
    height: int
    channels: int
    pixel_type: int
    top: int
    left: int
    
    def __init__(self) -> None: ...
    def to_numpy(self) -> NDArray[Any]: ...

class Dng:
    """DNG file handler"""
    def __init__(self) -> None: ...
    def read(self, path: str, ignore_enhanced: bool = False) -> int: ...
    def write(self, path: str) -> int: ...
    def get_data(self, enhanced: bool = False) -> DngData: ...
    def set_data(self, data: NDArray[Any], pixel_type: int, enhanced: bool = False) -> None: ...
    def get_baseline_exposure(self) -> float: ...
    def set_baseline_exposure(self, exposure: float) -> None: ...
    def get_meta(self) -> DngMeta: ...
    def set_meta(self, meta: DngMeta) -> None: ...

