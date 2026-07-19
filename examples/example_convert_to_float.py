"""
Convert a DNG image to float32 and save as a new DNG file.

Instead of creating a new DNG from scratch (which would lose camera profiles,
color matrices, and other essential metadata because SetMeta is currently a
no-op), this demo reuses the original DNG object and only replaces its image
data.  All metadata from the source file is preserved.
"""

import sys
import os
import numpy as np

try:
    import dngpy
except ImportError:
    print("Error: dngpy not found. Build and install the package first.")
    sys.exit(1)


def convert_to_float32(input_path, output_path):
    # 1. Read the source DNG — this populates the internal negative with
    #    all metadata (camera profiles, color matrices, EXIF, etc.)
    print(f"Reading: {input_path}")
    try:
        dng = dngpy.Dng(input_path, ignore_enhanced=False)
    except RuntimeError as e:
        print(f"Failed to open DNG: {e}")
        return

    meta = dng.metadata
    print(f"  Source: {meta.make} {meta.model}")
    print(f"  Dimensions: {meta.width} x {meta.height}")
    print(f"  Color planes: {meta.color_planes}, Space: {meta.color_space}")

    # 2. Get Stage1 image data and convert to float32
    raw = dng.raw_pixels
    print(f"  Raw array: shape={raw.shape}, dtype={raw.dtype}")

    NORM_TABLE = {
        np.dtype("uint8"):   (255.0,          "8-bit"),
        np.dtype("uint16"):  (65535.0,        "16-bit"),
        np.dtype("uint32"):  (4294967295.0,   "32-bit"),
        np.dtype("int16"):   (32767.0,        "16-bit signed"),
        np.dtype("float32"): (1.0,            "float"),
    }

    _, label = NORM_TABLE.get(raw.dtype, (1.0, "unknown"))

    if raw.dtype == np.int16:
        # Map [-32768, 32767] → [0.0, 1.0]
        raw_float = (raw.astype(np.float32) + 32768.0) / 65535.0
    elif raw.dtype == np.float32:
        raw_float = raw.copy()
    else:
        raw_float = raw.astype(np.float32)

    print(f"  Converted {label} → float32  shape={raw_float.shape}  "
          f"range=[{raw_float.min():.4f}, {raw_float.max():.4f}]")

    # 3. Replace image data on the *same* Dng object.
    #    This keeps all metadata from the original file intact.
    dng.set_raw_pixels(raw_float)

    # 4. Write
    print(f"Writing: {output_path}")
    try:
        dng.save(output_path)
        size_mb = os.path.getsize(output_path) / (1024 * 1024)
        print(f"  Done — {size_mb:.1f} MB written.")
    except RuntimeError as error:
        print(f"  Write failed: {error}")


if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    repo_root = os.path.dirname(script_dir)

    input_file = os.path.join(repo_root, "extern", "sample_files",
                              "01_jxl_linear_raw_integer.dng")
    output_file = os.path.join(repo_root, "output_float32.dng")

    if not os.path.exists(input_file):
        print(f"Input file not found: {input_file}")
        sys.exit(1)

    convert_to_float32(input_file, output_file)
