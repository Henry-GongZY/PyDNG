"""
Type stubs for the compiled ``pydng._native`` extension (pybind11).
"""

from typing import Any, overload
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

class DngGainMap:
    """DNG gain map"""
    rows: int
    cols: int
    planes: int
    spacing_v: float
    spacing_h: float
    origin_v: float
    origin_h: float
    data: NDArray[Any]

    def __init__(self) -> None: ...
    def to_numpy(self) -> NDArray[Any]: ...

class Dng:
    """DNG file handler"""

    def __init__(self, path: str, ignore_enhanced: bool = False) -> None: ...

    def _save(self, path: str) -> int: ...
    def _get_pixels(self, enhanced: bool = False) -> NDArray[Any]: ...
    def _get_data_info(self, enhanced: bool = False) -> DngData: ...
    def _set_pixels(self, data: NDArray[Any], pixel_type: int, enhanced: bool = False) -> None: ...
    def get_baseline_exposure(self) -> float: ...
    def set_baseline_exposure(self, exposure: float) -> None: ...
    def _get_metadata(self) -> DngMeta: ...
    def _get_exif(self) -> DngMeta: ...
    def _get_image_info(self) -> DngMeta: ...
    def _get_color_info(self) -> DngMeta: ...
    def set_meta(self, meta: DngMeta) -> None: ...
    def get_white_balance(self) -> list[float]: ...
    def set_white_balance(self, wb: list[float]) -> None: ...
    def get_gainmap(self) -> DngGainMap: ...
    def set_gainmap(self, map: DngGainMap) -> None: ...
    def get_bayer_pattern(self) -> str: ...
    def set_bayer_pattern(self, pattern: str) -> None: ...

