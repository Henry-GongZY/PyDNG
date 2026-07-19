"""Regression tests for the NumPy-first DNG API."""

import gc
import os
from pathlib import Path
import tempfile
import unittest

import numpy as np

import dngpy


ROOT = Path(__file__).resolve().parents[1]
SAMPLE_FILE = ROOT / "extern" / "sample_files" / "01_jxl_linear_raw_integer.dng"
PIXEL_TYPE_CODES = {
    np.dtype(np.uint8): 1,
    np.dtype(np.uint16): 3,
    np.dtype(np.int16): 8,
    np.dtype(np.uint32): 4,
    np.dtype(np.float32): 11,
}


class TestNumpyFirstApi(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not SAMPLE_FILE.is_file():
            raise unittest.SkipTest(f"sample DNG is unavailable: {SAMPLE_FILE}")

    def new_dng(self):
        return dngpy.Dng(str(SAMPLE_FILE))

    def test_set_raw_pixels_infers_supported_dtypes(self):
        for dtype, pixel_type in PIXEL_TYPE_CODES.items():
            with self.subTest(dtype=dtype):
                pixels = np.arange(18, dtype=dtype).reshape(2, 3, 3)
                dng = self.new_dng()

                dng.set_raw_pixels(pixels)

                info = dng.get_data_info()
                self.assertEqual((info.height, info.width, info.channels), pixels.shape)
                self.assertEqual(info.pixel_type, pixel_type)
                np.testing.assert_array_equal(dng.raw_pixels, pixels)

    def test_raw_pixels_returns_independent_array(self):
        pixels = np.arange(12, dtype=np.uint16).reshape(2, 2, 3)
        dng = self.new_dng()
        dng.set_raw_pixels(pixels)

        returned = dng.raw_pixels
        returned[0, 0, 0] = 999

        self.assertNotEqual(dng.raw_pixels[0, 0, 0], returned[0, 0, 0])

    def test_set_raw_pixels_can_replace_enhanced_pixels(self):
        dng = self.new_dng()
        raw = dng.raw_pixels
        sample_rows = np.array([0, raw.shape[0] // 2, raw.shape[0] - 1])
        sample_cols = np.array([0, raw.shape[1] // 2, raw.shape[1] - 1])
        expected_samples = raw[sample_rows, sample_cols].copy()

        dng.set_raw_pixels(raw, enhanced=True)

        enhanced = dng.enhanced_pixels
        self.assertEqual(enhanced.dtype, raw.dtype)
        self.assertEqual(enhanced.shape, raw.shape)
        np.testing.assert_array_equal(
            enhanced[sample_rows, sample_cols], expected_samples
        )

    def test_set_raw_pixels_rejects_non_contiguous_input(self):
        dng = self.new_dng()
        pixels = np.zeros((2, 3, 3), dtype=np.uint16)

        with self.assertRaises(ValueError):
            dng._set_pixels(pixels, 11, enhanced=False)

        with self.assertRaises(ValueError):
            dng.set_raw_pixels(np.asfortranarray(pixels))

    def test_only_constructor_and_new_api_are_public(self):
        with self.assertRaises(TypeError):
            dngpy.Dng()

        dng = self.new_dng()
        for name in (
            "read",
            "write",
            "get_data",
            "set_data",
            "get_meta",
            "get_image_info",
            "get_exif",
            "get_color_info",
        ):
            with self.subTest(name=name):
                self.assertFalse(hasattr(dng, name))

    def test_read_error_includes_operation_path_and_error_code(self):
        missing_path = ROOT / "does-not-exist.dng"

        with self.assertRaisesRegex(
            RuntimeError,
            r'Dng: failed to read ".*does-not-exist\.dng" \(error code \d+\)',
        ):
            dngpy.Dng(str(missing_path))

    def test_write_error_includes_operation_path_and_error_code(self):
        dng = self.new_dng()

        with tempfile.TemporaryDirectory() as temp_dir:
            output_path = Path(temp_dir) / "missing" / "output.dng"
            with self.assertRaisesRegex(
                RuntimeError,
                r'Dng: failed to write ".*output\.dng" \(error code \d+\)',
            ):
                dng.save(str(output_path))


class TestSampleDngIntegration(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        if not SAMPLE_FILE.is_file():
            raise unittest.SkipTest(f"sample DNG is unavailable: {SAMPLE_FILE}")

    def test_sample_read_and_info_api(self):
        dng = dngpy.Dng(str(SAMPLE_FILE))
        raw = dng.raw_pixels
        info = dng.get_data_info()

        self.assertEqual(raw.dtype, np.dtype(np.uint16))
        self.assertEqual((info.height, info.width, info.channels), raw.shape)
        self.assertEqual(info.pixel_type, 3)
        self.assertFalse(hasattr(info, "to_numpy"))

        self.assertEqual(dng.image_info.raw_width, raw.shape[1])
        self.assertEqual(dng.image_info.raw_height, raw.shape[0])
        self.assertIsInstance(dng.exif, dngpy.DngMeta)
        self.assertEqual(dng.color_info.color_planes, raw.shape[2])

    @unittest.skipUnless(
        os.environ.get("DNGPY_RUN_DNG_WRITE_TEST") == "1",
        "set DNGPY_RUN_DNG_WRITE_TEST=1 to run the large DNG write round trip",
    )
    def test_float32_write_round_trip(self):
        dng = dngpy.Dng(str(SAMPLE_FILE))
        raw = dng.raw_pixels
        converted = np.ascontiguousarray(raw.astype(np.float32))
        dng.set_raw_pixels(converted)

        sample_rows = np.array([0, raw.shape[0] // 2, raw.shape[0] - 1])
        sample_cols = np.array([0, raw.shape[1] // 2, raw.shape[1] - 1])
        expected_samples = converted[sample_rows, sample_cols].copy()

        with tempfile.TemporaryDirectory() as temp_dir:
            output_path = Path(temp_dir) / "roundtrip_float32.dng"
            self.assertEqual(dng.save(str(output_path)), 0)

            del raw
            del converted
            del dng
            gc.collect()

            reread = dngpy.Dng(str(output_path))
            reread_pixels = reread.raw_pixels
            self.assertEqual(reread_pixels.dtype, np.dtype(np.float32))
            np.testing.assert_array_equal(
                reread_pixels[sample_rows, sample_cols], expected_samples
            )


if __name__ == "__main__":
    unittest.main()
