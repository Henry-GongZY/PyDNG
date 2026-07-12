"""
Example: Reading a DNG file and extracting metadata, image data, and gain map
"""

import sys
import os

try:
    import dngpy
    import numpy as np
except ImportError:
    print("Error: Could not import dngpy. Make sure the module is built and installed.")
    sys.exit(1)

def read_dng_example(file_path):
    """Read a DNG file and display its information"""
    print(f"Reading DNG file: {file_path}")
    
    try:
        # Load the DNG file
        dng = dngpy.Dng(file_path, ignore_enhanced=False)
    except (RuntimeError, TypeError) as e:
        print(f"Error reading DNG file: {e}")
        return

    print("Successfully read DNG file!\n")
    
    # --- 1. Get Metadata ---
    meta = dng.metadata
    print("=== DNG Metadata ===")
    print(f"Make: {meta.make}")
    print(f"Model: {meta.model}")
    print(f"Software: {meta.software}")
    print(f"Image Size: {meta.width} x {meta.height}")
    print(f"RAW Size: {meta.raw_width} x {meta.raw_height}")
    print("Camera Settings:")
    print(f"  Exposure Time: {meta.exposure_time} sec")
    print(f"  F-Number: f/{meta.f_number}")
    print(f"  ISO: {meta.iso}")
    print(f"Color Space: {meta.color_space}")
    print("===================\n")
    
    # --- 2. Get Image Data (Stage 1 - Raw Data) ---
    print("Reading Stage 1 (raw) image data...")
    numpy_array = dng.raw_pixels
    data = dng.get_data_info(enhanced=False)
    
    print(f"Data shape: {data.height} x {data.width} x {data.channels}")
    print(f"Pixel type: {data.pixel_type}")
    print(f"Numpy array shape: {numpy_array.shape}")
    print(f"Numpy array dtype: {numpy_array.dtype}\n")
    
    # --- 3. Get Enhanced Image Data (Stage 3) if available ---
    try:
        print("Reading Stage 3 (enhanced) image data...")
        enhanced_array = dng.enhanced_pixels
        print(f"Enhanced numpy array shape: {enhanced_array.shape}\n")
    except Exception as e:
        print(f"Enhanced image not available: {e}\n")
    
    # --- 4. Get Baseline Exposure & White Balance ---
    baseline_exposure = dng.get_baseline_exposure()
    print(f"Baseline Exposure: {baseline_exposure}")
    
    wb = dng.get_white_balance()
    if wb:
        print(f"White Balance (Neutral): {wb}")
    print()

    # --- 5. Get Gain Map (Shading Correction) ---
    print("Checking for Gain Map...")
    gainmap = dng.get_gainmap()
    if gainmap:
        print(f"Gain Map Found: {gainmap.rows}x{gainmap.cols}, planes: {gainmap.planes}")
        
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
        print(f"Using default file: {os.path.abspath(file_path)}")
    
    if not os.path.exists(file_path):
        print(f"Error: File not found: {file_path}")
        sys.exit(1)
    
    read_dng_example(file_path)
