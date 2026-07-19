# PyDNG — Python bindings for the Adobe DNG SDK

[中文版](./README_zh.md)


This project provides Python bindings for the Adobe DNG SDK so you can read and write DNG (Digital Negative) files from Python.

## Features

- Read DNG files
- Write DNG files
- Extract metadata (EXIF, camera info, and more)
- Access image data (Stage1 raw and Stage3 enhanced)
- Set image data from NumPy arrays
- Read and write baseline exposure, white balance
- Gain map (shading correction) support
- Bayer pattern detection and configuration
- Works naturally with NumPy

## Repository layout

- `src/dngpy/` — installable Python package (`__init__.py`, type stubs, `py.typed`).
- `bindings/` — C++ layer for the pybind11 extension (`_native`) and `dng_validate`:
  - `include/` — headers (`dng.h`, `utils.h`, `pch.h`)
  - `src/` — `pydng_bindings.cpp`, `dng.cpp`
  - `main.cpp` — entry point for the `dng_validate` utility.
- `extern/` — vendored SDKs and third-party code (DNG SDK, XMP, libjxl, pybind11, libjpeg, sample DNG files).
- `cmake/` — CMake helper scripts (source lists, libjxl config, copy scripts).

### CI packaging

On [published releases](.github/workflows/build.yml), [GitHub Actions](.github/workflows/build.yml) uses a two-stage pipeline:
1.  **Stage 1**: Build the core `dng` shared library for Linux (manylinux2014), Windows (MSVC), and macOS (arm64).
2.  **Stage 2**: Use `cibuildwheel` to build Python wheels for all compatible versions (Python 3.8+) using the pre-built core library.

This ensures efficient build times and broad compatibility.

## Quick start

### Install with pip (recommended)

```bash
# Install from the project root
pip install .

# Editable install (development)
pip install -e .

# Install from PyPI (when available)
pip install dngpy
```

pip pulls in build dependencies and drives the CMake build for you.

### Manual build

Use a manual CMake workflow if you need full control over configuration and compilation.

## Requirements

- CMake 3.15 or newer
- Python 3.8 or newer (including development headers for the interpreter you build against)
- A C++14-capable compiler (GCC 4.9+ on Linux, Clang 3.4+ on macOS, MSVC 2015+ on Windows)
- pybind11 (vendored in `extern/pybind11`; fetched automatically if missing)

### Standard build (Recommended)

To build everything (the core library and the Python extension) at once for your current Python environment:

```bash
pip install .
```

### Advanced: Separated build

If you want to build the core library once and then build the bindings later (similar to the CI process):

#### 1. Build the core library

```bash
mkdir build
cd build
cmake .. -DBUILD_PYTHON_BINDINGS=OFF -DBUILD_DNG_VALIDATE=ON -DCMAKE_INSTALL_PREFIX=../install_dir
cmake --build . --target install --config Release
```

#### 2. Build the Python bindings using the pre-built core

```bash
cd ..
# Point to the install_dir from the previous step
pip install . --config-settings=cmake.args="-DBUILD_DNG_LIBRARY=OFF -DPREBUILT_DNG_PATH=./install_dir"
```

### Building dng_validate for C++ verification

```bash
mkdir build
cd build
cmake .. -DBUILD_DNG_VALIDATE=ON -DBUILD_PYTHON_BINDINGS=OFF
cmake --build . --config Release --target dng_validate
```

The executable will be located in the `build` directory (or `build/Release` on Windows).

## Usage

### Basic example — read a DNG

```python
import dngpy
import numpy as np

# Load from path (raises RuntimeError on failure)
dng = dngpy.Dng("input.dng")

pixels = dng.raw_pixels
info = dng.image_info
exif = dng.exif
color = dng.color_info

print(f"Camera: {exif.make} {exif.model}")
print(f"Image size: {info.width} x {info.height}")
print(f"Color: {color.color_space}")
print(f"Pixel array: {pixels.shape}, {pixels.dtype}")
```

### Write a DNG

```python
import dngpy
import numpy as np

height, width, channels = 1000, 1500, 3
image_data = np.random.randint(0, 65535, size=(height, width, channels), dtype=np.uint16)

dng = dngpy.Dng("template.dng")

dng.set_raw_pixels(image_data)

dng.set_baseline_exposure(0.5)
dng.set_white_balance([1.0, 1.0, 1.0])

try:
    dng.save("output.dng")
except RuntimeError as error:
    print(f"Write failed: {error}")
```

## API reference

### Class `Dng`

Main entry point for reading and writing DNG files.

#### Constructor

- `Dng(path: str, ignore_enhanced: bool = False)` — load `path` immediately; raises `RuntimeError` on failure.

#### Methods

- `raw_pixels -> np.ndarray`
  Stage 1 RAW pixels as an independent array.

- `enhanced_pixels -> np.ndarray`
  Stage 3 enhanced pixels as an independent array, when present.

- `metadata -> DngMeta`
  Combined EXIF, image, and color metadata.

- `image_info -> DngMeta`
  Image dimensions and crop geometry.

- `exif -> DngMeta`
  EXIF metadata.

- `color_info -> DngMeta`
  Color planes and color space.

- `set_raw_pixels(pixels: np.ndarray, enhanced: bool = False) -> None`
  Replace Stage 1 RAW pixels, or Stage 3 pixels when `enhanced=True`, inferring pixel type from NumPy dtype.

- `save(path: str) -> int`
  Save the DNG. Returns `ErrorCode.NONE` on success and raises `RuntimeError`
  with the path and DNG SDK error code on failure.

- `get_data_info(enhanced: bool = False) -> DngData`
  Return image dimensions, channel count, pixel type, and active-area offset without exposing a raw buffer.

- `get_baseline_exposure() -> float`  
  Baseline exposure value.

- `set_baseline_exposure(exposure: float) -> None`  
  Set baseline exposure.

- `get_white_balance() -> list[float]`  
  Get white balance neutral vector (e.g., `[r, g, b]` gains).

- `set_white_balance(wb: list[float]) -> None`  
  Set white balance neutral vector.

- `get_gainmap() -> DngGainMap | None`  
  Get gain map (shading correction) if available.

- `set_gainmap(map: DngGainMap) -> None`  
  Set gain map.

- `get_bayer_pattern() -> str`  
  For a 2x2 rectangular RGB CFA, returns `"RGGB"`, `"GRBG"`, `"BGGR"`, or `"GBRG"` (row-major); otherwise `""`.

- `set_bayer_pattern(pattern: str) -> None`  
  Sets the 2x2 Bayer phase; `pattern` must be one of those four strings (case-insensitive).

### Pixel Types

Inferred by `set_raw_pixels()` from the NumPy dtype:

| String        | Code | DNG Constant | C Type     |
|---------------|------|-------------|------------|
| `"uint8"`     | 1    | ttByte      | uint8_t    |
| `"uint16"`    | 3    | ttShort     | uint16_t   |
| `"int16"`     | 8    | ttSShort    | int16_t    |
| `"uint32"`    | 4    | ttLong      | uint32_t   |
| `"float32"`   | 11   | ttFloat     | float      |

### Class `DngMeta`

Metadata for a DNG file.

#### Fields

| Field              | Type   | Description                  |
|--------------------|--------|------------------------------|
| `make`             | str    | Camera manufacturer          |
| `model`            | str    | Camera model                 |
| `software`         | str    | Software string              |
| `artist`           | str    | Artist / author              |
| `copyright`        | str    | Copyright notice             |
| `width`            | int    | Image width (cropped)        |
| `height`           | int    | Image height (cropped)       |
| `raw_width`        | int    | Raw image width              |
| `raw_height`       | int    | Raw image height             |
| `exposure_time`    | float  | Exposure time (seconds)      |
| `f_number`         | float  | Aperture f-number            |
| `focal_length`     | float  | Focal length (mm)            |
| `iso`              | int    | ISO sensitivity              |
| `focal_length_35mm`| int    | 35mm equivalent focal length |
| `date_time`        | str    | Capture timestamp            |
| `date_time_original`| str   | Original capture timestamp   |
| `is_monochrome`    | bool   | Monochrome flag              |
| `color_planes`     | int    | Number of color planes       |
| `color_space`      | str    | Color space name             |

### Class `DngData`

Image description returned by `get_data_info()`.

#### Fields

- `width`, `height`, `channels` — layout
- `pixel_type` — internal type code (see Pixel Types table)
- `top`, `left` — active-area offset

Use `Dng.raw_pixels` to retrieve pixel values.

### Class `DngGainMap`

Gain map for shading correction, returned by `get_gainmap()`.

#### Fields

- `rows`, `cols`, `planes` — grid dimensions
- `spacing_v`, `spacing_h` — spacing between grid points
- `origin_v`, `origin_h` — grid origin
- `data` — NumPy array of gain values

#### Methods

- `to_numpy() -> np.ndarray` — Export gain map data as a NumPy array.

### Constants — `ErrorCode`

- `NONE = 0` — success
- `READ_FILE` — read failure
- `WRITE_FILE` — write failure
- `BAD_FORMAT` — invalid format
- `UNKNOWN` — other error

## Examples

See the `examples/` directory:

- `example_read_dng.py` — load a DNG and print metadata, image data, gain map
- `example_write_dng.py` — build and write a synthetic DNG

Sample DNG files for testing are in `extern/sample_files/`.

## Notes

1. **Memory** — `raw_pixels` and `enhanced_pixels` return independent NumPy-owned arrays. They are safe to retain after the `Dng` object is released.
2. **Pixel types** — `set_raw_pixels()` infers the pixel type from the array dtype. Supported dtypes are listed above.
3. **Layout** — Image arrays are expected as `(height, width, channels)` in C-contiguous order.
4. **Windows paths** — Paths are handled with wide-character APIs where required.

## Tests

After building `_native`, run the default API and sample-read tests with:

```powershell
$env:PYTHONPATH = "$PWD\build\python"
python -m unittest discover -s tests -v
```

To include the large float32 DNG write round trip:

```powershell
$env:PYTHONPATH = "$PWD\build\python"
$env:DNGPY_RUN_DNG_WRITE_TEST = "1"
python -m unittest discover -s tests -v
```
5. **Call order when writing** — Call `set_raw_pixels()` BEFORE other setters (`set_baseline_exposure`, `set_white_balance`, etc.) as it initializes the internal negative.

## Troubleshooting

### Import errors

1. Confirm the extension module built successfully.
2. Verify the Python version matches the compiled `.pyd`/`.so` suffix (e.g., `cp313` = Python 3.13).
3. On Windows, `dng.dll` must be in the same folder as `_native.pyd` (or on `PATH`).

### Build failures

1. Install the Python development package for your interpreter (headers and libs).
2. Verify CMake finds the intended Python (`Python3_ROOT`, `CMAKE_PREFIX_PATH`, etc.).
3. Confirm you have a working C++14 toolchain.
4. On Unix, `libjpeg` and `libjxl` are built as external projects and require autotools / CMake.

## License

This project builds on the Adobe DNG SDK; use and redistribution must comply with the Adobe license terms that apply to the SDK and to this repository.
