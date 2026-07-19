"""
Example: Replacing pixels in a template DNG and writing a new DNG file
"""

import sys
import os

try:
    import dngpy
    import numpy as np
except ImportError:
    print("Error: Could not import dngpy. Make sure the module is built and installed.")
    sys.exit(1)

def write_dng_example(template_path, output_path):
    print("Generating synthetic image data...")
    # Create a dummy image: 1000x1500 pixels, 3 channels (RGB)
    height, width, channels = 1000, 1500, 3
    
    # 16-bit unsigned integer data
    image_data = np.random.randint(0, 65535, size=(height, width, channels), dtype=np.uint16)
    
    # Create a gradient for a nicer visual effect instead of pure noise
    gradient = np.linspace(0, 65535, width, dtype=np.uint16)
    image_data[:, :, 0] = gradient  # Red channel gradient
    
    print(f"Created image array: shape={image_data.shape}, dtype={image_data.dtype}")

    # Load a template DNG to preserve a valid DNG container and its metadata.
    dng = dngpy.Dng(template_path)

    # --- 1. Set Image Data ---
    print("Setting image data into DNG...")
    dng.set_raw_pixels(image_data)

    # --- 2. Set White Balance and Exposure ---
    dng.set_baseline_exposure(0.5)
    dng.set_white_balance([1.0, 1.0, 1.0])

    # --- 3. Write to disk ---
    print(f"Writing DNG file to {output_path}...")
    try:
        dng.save(output_path)
        print("Success! DNG file written.")
    except RuntimeError as error:
        print(f"Failed to write DNG file: {error}")

if __name__ == "__main__":
    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    template_file = os.path.join(
        repo_root, "extern", "sample_files", "01_jxl_linear_raw_integer.dng"
    )
    output_file = "output_synthetic.dng"
    write_dng_example(template_file, output_file)
