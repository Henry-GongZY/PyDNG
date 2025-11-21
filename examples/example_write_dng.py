"""
Example: Creating and writing a DNG file from numpy array
"""

import sys
import os

# Add parent directory to path to import pydng
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'build', 'python'))

try:
    import pydng
    import numpy as np
except ImportError:
    print("Error: Could not import pydng. Make sure the module is built and installed.")
    sys.exit(1)

def write_dng_example(output_path):
    """Create a DNG file from numpy array"""
    
    # Create a sample image (16-bit RGB)
    height, width, channels = 1000, 1500, 3
    image_data = np.random.randint(0, 65535, size=(height, width, channels), dtype=np.uint16)
    
    print(f"Created sample image: {height} x {width} x {channels}")
    print(f"Data type: {image_data.dtype}")
    print()
    
    # Create Dng object
    dng = pydng.Dng()
    
    # Set image data
    # Pixel type constants: ttByte=1, ttShort=3, ttSShort=8, ttLong=4
    print("Setting image data...")
    dng.set_data(image_data, 3, enhanced=False)  # 3 = ttShort (16-bit unsigned)
    
    # Create and set metadata
    print("Setting metadata...")
    meta = pydng.DngMeta()
    meta.make = "PyDNG"
    meta.model = "Example Camera"
    meta.software = "PyDNG Example"
    meta.width = width
    meta.height = height
    meta.raw_width = width
    meta.raw_height = height
    meta.exposure_time = 1.0 / 60.0
    meta.f_number = 2.8
    meta.focal_length = 50.0
    meta.iso = 100
    meta.is_monochrome = False
    meta.color_planes = channels
    meta.color_space = "RGB"
    
    dng.set_meta(meta)
    
    # Set baseline exposure
    dng.set_baseline_exposure(0.0)
    
    # Write DNG file
    print(f"Writing DNG file to: {output_path}")
    error_code = dng.write(output_path)
    
    if error_code != pydng.ErrorCode.NONE:
        print(f"Error writing DNG file: {error_code}")
        return False
    
    print("Successfully wrote DNG file!")
    return True

if __name__ == "__main__":
    if len(sys.argv) < 2:
        output_path = "output_example.dng"
        print(f"No output path specified, using: {output_path}")
    else:
        output_path = sys.argv[1]
    
    write_dng_example(output_path)

