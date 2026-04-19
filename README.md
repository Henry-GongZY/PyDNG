# PyDNG — Python bindings for the Adobe DNG SDK

This project provides fundamental Python bindings for the Adobe DNG SDK so you can read and write DNG (Digital Negative) files from Python.

## Features

- Read DNG files
- Write DNG files
- Extract metadata (EXIF, camera info, and more)
- Access image data (Stage1 and Stage3)
- Set image data from NumPy arrays
- Read and write baseline exposure
- Works naturally with NumPy

## Repository layout

- `src/pydng/` — installable Python package (`__init__.py`, type stubs, `py.typed`).
- `bindings/` — C++ layer for the pybind11 extension (`_native`) and `dng_validate`:
  - `include/` — headers (`dng.h`, `utils.h`, `pch.h`)
  - `src/` — `pydng_bindings.cpp`, `dng.cpp`
  - `main.cpp` — entry point for the `dng_validate` utility.
- `extern/` — vendored SDKs and third-party code (DNG SDK, XMP, libjxl, pybind11, etc.).

### CI packaging

On pushes and pull requests, [GitHub Actions](.github/workflows/build.yml) uses a two-stage pipeline:
1.  **Stage 1**: Build the core `dng` shared library for Linux and Windows.
2.  **Stage 2**: Use `cibuildwheel` to build Python wheels for all compatible versions (Python 3.8 to 3.12) using the pre-built core library.

This ensures efficient build times and broad compatibility.

## Quick start

### Install with pip (recommended)

The simplest approach is a one-step install:

```bash
# Install from the project root
pip install .

# Editable install (development)
pip install -e .

# Install from a Git repository
pip install git+https://github.com/yourusername/PyDNG.git
```

pip pulls in build dependencies and drives the CMake build for you.

### Manual build

Use a manual CMake workflow if you need full control over configuration and compilation.

## Requirements

- CMake 3.15 or newer
- Python 3.8 or newer (including development headers for the interpreter you build against)
- A C++14-capable compiler (GCC 4.9 or newer on Linux, Clang 3.4 or newer on macOS, MSVC 2015 or newer on Windows)
- pybind11 (via the `extern/pybind11` git submodule, or fetched automatically if missing)

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

To build the `dng_validate` command-line tool for verifying your C++ changes:

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
import pydng
import numpy as np

# Load from path (raises RuntimeError on failure)
dng = pydng.Dng("input.dng", ignore_enhanced=False)

meta = dng.get_meta()
print(f"Camera: {meta.make} {meta.model}")
print(f"Image size: {meta.width} x {meta.height}")
print(f"ISO: {meta.iso}")
print(f"Exposure time: {meta.exposure_time} s")

data = dng.get_data(enhanced=False)
numpy_array = data.to_numpy()
print(f"Image shape: {numpy_array.shape}")
```

You can still use `dng = pydng.Dng()` followed by `dng.read(path)` if you prefer checking `ErrorCode` instead of exceptions.

### Write a DNG

```python
import pydng
import numpy as np

height, width, channels = 1000, 1500, 3
image_data = np.random.randint(0, 65535, size=(height, width, channels), dtype=np.uint16)

dng = pydng.Dng()

# 3 = ttShort (16-bit unsigned)
dng.set_data(image_data, 3, enhanced=False)

meta = pydng.DngMeta()
meta.make = "My Camera"
meta.model = "Example"
meta.width = width
meta.height = height
meta.iso = 100
meta.exposure_time = 1.0 / 60.0
meta.f_number = 2.8
meta.focal_length = 50.0

dng.set_meta(meta)

error_code = dng.write("output.dng")
```

## API reference

### Class `Dng`

Main entry point for reading and writing DNG files.

#### Constructor

- `Dng()` — empty object; use `read()` to load a file.
- `Dng(path: str, ignore_enhanced: bool = False)` — load `path` immediately; raises `RuntimeError` on failure (same behavior as `read()` returning a non-`NONE` code).

#### Methods

- `get_bayer_pattern() -> str`  
  For a 2×2 rectangular RGB CFA, returns `"RGGB"`, `"GRBG"`, `"BGGR"`, or `"GBRG"` (row-major tile); otherwise `""`.

- `set_bayer_pattern(pattern: str) -> None`  
  Sets the 2×2 Bayer phase; `pattern` must be one of those four strings (case-insensitive). Requires a 3-plane RGB CFA (or uninitialized mosaic, which is initialized with `SetRGB()`).

- `read(path: str, ignore_enhanced: bool = False) -> ErrorCode`  
  Load a DNG from disk (return code; no exception on error).

- `write(path: str) -> ErrorCode`  
  Save a DNG to disk.

- `get_data(enhanced: bool = False) -> DngData`  
  Return image data. `enhanced=True` selects Stage3; `False` selects Stage1.

- `set_data(data: np.ndarray, pixel_type: int, enhanced: bool = False) -> None`  
  Set image data. `data` has shape `(height, width, channels)`.  
  `pixel_type`: numeric code (`1` = ttByte, `3` = ttShort, `8` = ttSShort, `4` = ttLong).

- `get_meta() -> DngMeta`  
  Return metadata.

- `set_meta(meta: DngMeta) -> None`  
  Apply metadata.

- `get_baseline_exposure() -> float`  
  Baseline exposure value.

- `set_baseline_exposure(exposure: float) -> None`  
  Set baseline exposure.

### Class `DngMeta`

Metadata for a DNG file.

#### Fields

- `make`, `model`: camera make and model  
- `software`: software string  
- `artist`, `copyright`: attribution and rights  
- `width`, `height`: image dimensions  
- `raw_width`, `raw_height`: raw dimensions  
- `exposure_time`: exposure in seconds  
- `f_number`: aperture  
- `focal_length`: focal length in mm  
- `iso`: sensitivity  
- `focal_length_35mm`: 35 mm equivalent focal length  
- `date_time`, `date_time_original`: timestamps  
- `is_monochrome`: monochrome flag  
- `color_planes`, `color_space`: color layout and space  

### Class `DngData`

Image buffer returned by `get_data()`.

#### Fields

- `width`, `height`, `channels`: layout  
- `pixel_type`: internal type code  
- `top`, `left`: active-area offset  

#### Methods

- `to_numpy() -> np.ndarray`  
  Export as a NumPy array.

### Constants — `ErrorCode`

- `NONE`: success  
- `READ_FILE`: read failure  
- `WRITE_FILE`: write failure  
- `BAD_FORMAT`: invalid format  
- `UNKNOWN`: other error  

Pixel type codes (see also [PIXEL_TYPES.md](PIXEL_TYPES.md)):

- `1` — ttByte (8-bit unsigned)  
- `3` — ttShort (16-bit unsigned)  
- `8` — ttSShort (16-bit signed)  
- `4` — ttLong (32-bit unsigned)  

## Examples

See the `examples/` directory:

- `example_read_dng.py` — load a DNG and print information  
- `example_write_dng.py` — build and write a DNG  

## Notes

1. **Memory** — `DngData` lifetime is tied to the conversion to NumPy; do not try to manually free the underlying pointer.

2. **Pixel types** — Choose `pixel_type` in `set_data()` so it matches the dtype and layout of your array.

3. **Layout** — Image arrays are expected as `(height, width, channels)`.

4. **Windows paths** — Paths are handled with the appropriate wide-character APIs where required.

## Troubleshooting

### Import errors

1. Confirm the extension module built successfully.  
2. Ensure the build output is on `PYTHONPATH` or installed into site-packages.  
3. On Windows, native dependencies (`dng.dll` and related) must be discoverable (same folder as the `.pyd` or on `PATH`).

### Build failures

1. Install the Python development package for your interpreter (headers and libs).  
2. Verify CMake finds the intended Python (`Python3_ROOT`, `CMAKE_PREFIX_PATH`, etc.).  
3. Confirm you have a working C++14 toolchain.

## License

This project builds on the Adobe DNG SDK; use and redistribution must comply with the Adobe license terms that apply to the SDK and to this repository.
