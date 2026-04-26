"""
Example: Creating a synthetic image and writing it to a DNG file
"""

import sys
import os

try:
    import dngpy
    import numpy as np
except ImportError:
    print("Error: Could not import dngpy. Make sure the module is built and installed.")
    sys.exit(1)

def write_dng_example(output_path):
    print("Generating synthetic image data...")
    # Create a dummy image: 1000x1500 pixels, 3 channels (RGB)
    height, width, channels = 1000, 1500, 3
    
    # 16-bit unsigned integer data
    image_data = np.random.randint(0, 65535, size=(height, width, channels), dtype=np.uint16)
    
    # Create a gradient for a nicer visual effect instead of pure noise
    gradient = np.linspace(0, 65535, width, dtype=np.uint16)
    image_data[:, :, 0] = gradient  # Red channel gradient
    
    print(f"Created image array: shape={image_data.shape}, dtype={image_data.dtype}")

    # Initialize a new Dng object
    dng = dngpy.Dng()

    # --- 1. Set Image Data ---
    # The 'pixel_type' must match your numpy dtype
    # 1 = ttByte (8-bit), 3 = ttShort (16-bit), 4 = ttLong (32-bit), 8 = ttSShort (signed 16-bit)
    print("Setting image data into DNG...")
    dng.set_data(image_data, 3, enhanced=False)

    # --- 2. Set Metadata ---
    print("Populating metadata...")
    meta = dngpy.DngMeta()
    meta.make = "dngpy Camera"
    meta.model = "Synthetic Model 1"
    meta.software = "dngpy Python Bindings"
    meta.width = width
    meta.height = height
    meta.raw_width = width
    meta.raw_height = height
    meta.iso = 400
    meta.exposure_time = 1.0 / 125.0
    meta.f_number = 4.0
    meta.focal_length = 35.0
    meta.color_planes = channels
    meta.color_space = "sRGB"
    
    dng.set_meta(meta)

    # --- 3. Set White Balance and Exposure ---
    dng.set_baseline_exposure(0.5)
    dng.set_white_balance([1.0, 1.0, 1.0])

    # --- 4. Write to disk ---
    print(f"Writing DNG file to {output_path}...")
    error_code = dng.write(output_path)
    
    if error_code != dngpy.ErrorCode.NONE:
        print(f"Failed to write DNG file. Error code: {error_code}")
    else:
        print("Success! DNG file written.")

if __name__ == "__main__":
    output_file = "output_synthetic.dng"
    write_dng_example(output_file)
