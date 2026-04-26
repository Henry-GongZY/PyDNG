"""
Example: Reading a DNG file and extracting metadata and image data
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

def read_dng_example(file_path):
    """Read a DNG file and display its information"""
    try:
        dng = pydng.Dng(file_path, ignore_enhanced=False)
    except (RuntimeError, TypeError) as e:
        print(f"Error reading DNG file: {e}")
        return

    print("Successfully read DNG file!")
    print()
    
    # Get metadata
    meta = dng.get_meta()
    print("=== DNG Metadata ===")
    print(f"Make: {meta.make}")
    print(f"Model: {meta.model}")
    print(f"Software: {meta.software}")
    print(f"Artist: {meta.artist}")
    print(f"Copyright: {meta.copyright}")
    print()
    print(f"Image Size: {meta.width} x {meta.height}")
    print(f"RAW Size: {meta.raw_width} x {meta.raw_height}")
    print()
    print("Camera Settings:")
    print(f"  Exposure Time: {meta.exposure_time} sec")
    print(f"  F-Number: f/{meta.f_number}")
    print(f"  Focal Length: {meta.focal_length} mm")
    if meta.focal_length_35mm > 0:
        print(f"  35mm Equivalent: {meta.focal_length_35mm} mm")
    print(f"  ISO: {meta.iso}")
    print()
    print(f"DateTime: {meta.date_time}")
    print(f"DateTimeOriginal: {meta.date_time_original}")
    print()
    print(f"Monochrome: {meta.is_monochrome}")
    print(f"Color Planes: {meta.color_planes}")
    print(f"Color Space: {meta.color_space}")
    print("===================")
    print()
    
    # Get image data (Stage 1 - raw data)
    print("Reading Stage 1 (raw) image data...")
    data = dng.get_data(enhanced=False)
    print(f"Data shape: {data.height} x {data.width} x {data.channels}")
    print(f"Pixel type: {data.pixel_type}")
    print(f"Active area: top={data.top}, left={data.left}")
    
    # Convert to numpy array
    numpy_array = data.to_numpy()
    print(f"Numpy array shape: {numpy_array.shape}")
    print(f"Numpy array dtype: {numpy_array.dtype}")
    print()
    
    # Get enhanced image data (Stage 3) if available
    try:
        print("Reading Stage 3 (enhanced) image data...")
        enhanced_data = dng.get_data(enhanced=True)
        if enhanced_data:
            print(f"Enhanced data shape: {enhanced_data.height} x {enhanced_data.width} x {enhanced_data.channels}")
            enhanced_array = enhanced_data.to_numpy()
            print(f"Enhanced numpy array shape: {enhanced_array.shape}")
    except Exception as e:
        print(f"Enhanced image not available: {e}")
    
    # Get baseline exposure
    baseline_exposure = dng.get_baseline_exposure()
    print(f"Baseline Exposure: {baseline_exposure}")
    print()

    # Get Gainmap
    print("Checking for Gain Map (shading correction)...")
    gainmap = dng.get_gainmap()
    if gainmap:
        print(f"Gain Map Found: {gainmap.rows}x{gainmap.cols}, planes: {gainmap.planes}")
        print(f"  Spacing: v={gainmap.spacing_v}, h={gainmap.spacing_h}")
        print(f"  Origin: v={gainmap.origin_v}, h={gainmap.origin_h}")
        
        # Access data as numpy array
        gm_data = gainmap.to_numpy()
        print(f"Gain Map data shape: {gm_data.shape}")
        
        # Save to local file
        output_name = "gainmap.npy"
        np.save(output_name, gm_data)
        print(f"Gain Map data saved to {output_name}")
    else:
        print("No Gain Map found in this DNG.")
    
    return dng, meta, numpy_array

if __name__ == "__main__":
    # Default path relative to the script location
    default_path = os.path.join(os.path.dirname(__file__), '..', 'extern', 'sample_files', '01_jxl_linear_raw_integer.dng')
    
    if len(sys.argv) >= 2:
        file_path = sys.argv[1]
    else:
        file_path = default_path
        print(f"Using default file: {file_path}")
    
    if not os.path.exists(file_path):
        print(f"Error: File not found: {file_path}")
        sys.exit(1)
    
    read_dng_example(file_path)

