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

## Build instructions

### Windows

```bash
mkdir build
cd build

cmake .. -DBUILD_PYTHON_BINDINGS=ON
cmake --build . --config Release
```

The built Python extension ends up under the build tree (e.g. `build/python/` depending on your CMake settings).

### Linux / macOS

```bash
mkdir build
cd build

cmake .. -DBUILD_PYTHON_BINDINGS=ON -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

On Unix, the module may also be copied to your user site-packages by the build; see `CMakeLists.txt` for details.

## Usage

### Basic example — read a DNG

```python
import pydng
import numpy as np

dng = pydng.Dng()

error_code = dng.read("input.dng", ignore_enhanced=False)

if error_code == pydng.ErrorCode.NONE:
    meta = dng.get_meta()
    print(f"Camera: {meta.make} {meta.model}")
    print(f"Image size: {meta.width} x {meta.height}")
    print(f"ISO: {meta.iso}")
    print(f"Exposure time: {meta.exposure_time} s")

    data = dng.get_data(enhanced=False)
    numpy_array = data.to_numpy()
    print(f"Image shape: {numpy_array.shape}")
```

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

#### Methods

- `read(path: str, ignore_enhanced: bool = False) -> ErrorCode`  
  Load a DNG from disk.

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
