//
// Python bindings for PyDNG using pybind11
//

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <pybind11/functional.h>
#include <cstring>
#include "dng.h"

namespace py = pybind11;

// Wrapper class for error codes (dng_error_code is typedef int32, not enum)
struct ErrorCode {
    static const dng_error_code NONE;
    static const dng_error_code READ_FILE;
    static const dng_error_code WRITE_FILE;
    static const dng_error_code BAD_FORMAT;
    static const dng_error_code UNKNOWN;
};

const dng_error_code ErrorCode::NONE = dng_error_none;
const dng_error_code ErrorCode::READ_FILE = dng_error_read_file;
const dng_error_code ErrorCode::WRITE_FILE = dng_error_write_file;
const dng_error_code ErrorCode::BAD_FORMAT = dng_error_bad_format;
const dng_error_code ErrorCode::UNKNOWN = dng_error_unknown;

py::dtype DtypeForPixelType(uint32_t pixel_type) {
    switch (pixel_type) {
        case ttByte: return py::dtype::of<uint8_t>();
        case ttShort: return py::dtype::of<uint16_t>();
        case ttSShort: return py::dtype::of<int16_t>();
        case ttLong: return py::dtype::of<uint32_t>();
        case ttFloat: return py::dtype::of<float>();
        default: throw std::invalid_argument("Unsupported DNG pixel type");
    }
}

// Return an independent NumPy-owned copy. No C++ buffer or capsule survives this call.
py::array DngDataToNumpy(const DngData& data) {
    if (!data.Data()) {
        throw std::runtime_error("DNG image data is empty");
    }
    py::array result(DtypeForPixelType(data.pixel_type),
                     {data.height, data.width, data.channels});
    std::memcpy(result.mutable_data(), data.Data(), data.bytes.size());
    return result;
}

DngData NumpyToDngData(const py::array& arr, uint32_t pixel_type) {
    if (arr.ndim() != 3) {
        throw std::invalid_argument("Array must have shape (height, width, channels)");
    }
    if (!(arr.flags() & py::array::c_style)) {
        throw std::invalid_argument("Array must be C-contiguous");
    }
    if (!arr.dtype().is(DtypeForPixelType(pixel_type))) {
        throw std::invalid_argument("Array dtype does not match pixel_type");
    }

    DngData data;
    data.height = static_cast<uint32>(arr.shape(0));
    data.width = static_cast<uint32>(arr.shape(1));
    data.channels = static_cast<uint32>(arr.shape(2));
    data.pixel_type = pixel_type;
    data.bytes.resize(arr.nbytes());
    std::memcpy(data.Data(), arr.data(), data.bytes.size());
    return data;
}

// Helper to convert DngGainMap data to numpy array
py::array_t<float> DngGainMapToNumpy(const DngGainMap& map) {
    if (map.data.empty()) return py::array_t<float>();
    return py::array_t<float>(
        {map.rows, map.cols, map.planes},
        map.data.data()
    );
}

PYBIND11_MODULE(_native, m) {
    m.doc() = "PyDNG: low-level C++ extension (internal)";
    
    // Error codes (dng_error_code is a typedef int32, not an enum)
    // Create a simple ErrorCode class for convenience
    py::class_<ErrorCode>(m, "ErrorCode")
        .def_readonly_static("NONE", &ErrorCode::NONE)
        .def_readonly_static("READ_FILE", &ErrorCode::READ_FILE)
        .def_readonly_static("WRITE_FILE", &ErrorCode::WRITE_FILE)
        .def_readonly_static("BAD_FORMAT", &ErrorCode::BAD_FORMAT)
        .def_readonly_static("UNKNOWN", &ErrorCode::UNKNOWN);
    
    // Note: Pixel type constants are not exported.
    // Use numeric values: ttByte=1, ttShort=3, ttSShort=8, ttLong=4
    
    // DngMeta class
    py::class_<DngMeta>(m, "DngMeta")
        .def(py::init<>())
        .def_readwrite("make", &DngMeta::make)
        .def_readwrite("model", &DngMeta::model)
        .def_readwrite("software", &DngMeta::software)
        .def_readwrite("artist", &DngMeta::artist)
        .def_readwrite("copyright", &DngMeta::copyright)
        .def_readwrite("width", &DngMeta::width)
        .def_readwrite("height", &DngMeta::height)
        .def_readwrite("raw_width", &DngMeta::rawWidth)
        .def_readwrite("raw_height", &DngMeta::rawHeight)
        .def_readwrite("exposure_time", &DngMeta::exposureTime)
        .def_readwrite("f_number", &DngMeta::fNumber)
        .def_readwrite("focal_length", &DngMeta::focalLength)
        .def_readwrite("iso", &DngMeta::iso)
        .def_readwrite("focal_length_35mm", &DngMeta::focalLength35mm)
        .def_readwrite("date_time", &DngMeta::dateTime)
        .def_readwrite("date_time_original", &DngMeta::dateTimeOriginal)
        .def_readwrite("is_monochrome", &DngMeta::isMonochrome)
        .def_readwrite("color_planes", &DngMeta::colorPlanes)
        .def_readwrite("color_space", &DngMeta::colorSpace)
        .def("__repr__", [](const DngMeta& meta) {
            return "<DngMeta make='" + meta.make + "' model='" + meta.model + 
                   "' width=" + std::to_string(meta.width) + 
                   " height=" + std::to_string(meta.height) + ">";
        });
    
    // DngGainMap class
    py::class_<DngGainMap>(m, "DngGainMap")
        .def(py::init<>())
        .def_readwrite("rows", &DngGainMap::rows)
        .def_readwrite("cols", &DngGainMap::cols)
        .def_readwrite("planes", &DngGainMap::planes)
        .def_readwrite("spacing_v", &DngGainMap::spacingV)
        .def_readwrite("spacing_h", &DngGainMap::spacingH)
        .def_readwrite("origin_v", &DngGainMap::originV)
        .def_readwrite("origin_h", &DngGainMap::originH)
        .def_property("data", 
            [](const DngGainMap& self) { return DngGainMapToNumpy(self); },
            [](DngGainMap& self, py::array_t<float> array) {
                if (array.ndim() != 3) {
                    throw std::runtime_error("GainMap data must be 3-dimensional (rows, cols, planes)");
                }
                auto r = array.unchecked<3>();
                self.rows = (uint32_t)r.shape(0);
                self.cols = (uint32_t)r.shape(1);
                self.planes = (uint32_t)r.shape(2);
                self.data.assign(array.data(), array.data() + array.size());
            })
        .def("to_numpy", &DngGainMapToNumpy);
    
    // Image metadata / compatibility object. Pixel buffers are always RAII-owned.
    py::class_<DngData>(m, "DngData")
        .def(py::init<>())
        .def_readonly("width", &DngData::width)
        .def_readonly("height", &DngData::height)
        .def_readonly("channels", &DngData::channels)
        .def_readonly("pixel_type", &DngData::pixel_type)
        .def_readonly("top", &DngData::top)
        .def_readonly("left", &DngData::left)
        .def("__repr__", [](const DngData& data) {
            return "<DngData width=" + std::to_string(data.width) + 
                   " height=" + std::to_string(data.height) + 
                   " channels=" + std::to_string(data.channels) + ">";
        });
    
    // Dng class
    py::class_<Dng>(m, "Dng")
        .def(py::init([](const std::string& path, bool ignore_enhanced) {
                 return new Dng(path, ignore_enhanced);
             }),
             py::arg("path"),
             py::arg("ignore_enhanced") = false,
             "Load a DNG from path. Raises RuntimeError on failure.")
        .def("_save", &Dng::Write,
             py::arg("path"),
             "Internal save implementation. Raises RuntimeError on failure.")
        .def("_get_pixels", [](Dng& self, bool enhanced) {
            return DngDataToNumpy(self.GetData(enhanced));
        }, py::arg("enhanced") = false,
           "Internal pixel retrieval implementation")
        .def("_get_data_info", &Dng::GetDataInfo,
             py::arg("enhanced") = false,
             "Internal image layout retrieval implementation")
        .def("_set_pixels", [](Dng& self, py::array arr, uint32_t pixel_type, bool enhanced) {
            self.SetData(NumpyToDngData(arr, pixel_type), enhanced);
        }, py::arg("data"), py::arg("pixel_type"), py::arg("enhanced") = false,
           "Internal pixel update implementation")
        .def("get_baseline_exposure", &Dng::GetBaselineExposure,
             "Get baseline exposure value")
        .def("set_baseline_exposure", &Dng::SetBaselineExposure,
             py::arg("exposure"),
             "Set baseline exposure value")
        .def("_get_metadata", &Dng::GetMeta, "Internal metadata retrieval implementation")
        .def("_get_exif", &Dng::GetExif, "Internal EXIF retrieval implementation")
        .def("_get_image_info", &Dng::GetImageInfo, "Internal image-info retrieval implementation")
        .def("_get_color_info", &Dng::GetColorInfo, "Internal color-info retrieval implementation")
        .def("set_meta", &Dng::SetMeta,
             py::arg("meta"),
             "Set metadata for DNG file")
        .def("get_white_balance", &Dng::GetWhiteBalance,
             "Get white balance neutral vector (e.g., [r, g, b] gains)")
        .def("set_white_balance", &Dng::SetWhiteBalance,
             py::arg("wb"),
             "Set white balance neutral vector (e.g., [r, g, b] gains)")
        .def("get_gainmap", [](Dng& self) {
            auto map = self.GetGainmap();
            return std::unique_ptr<DngGainMap>(map);
        }, "Get gain map from DNG file")
        .def("set_gainmap", &Dng::SetGainmap,
             py::arg("map"),
             "Set gain map for DNG file")
        .def("get_bayer_pattern", &Dng::GetBayerPattern,
             "Return 2x2 Bayer tile as RGGB/GRBG/BGGR/GBRG, or empty string if not available")
        .def("set_bayer_pattern", &Dng::SetBayerPattern,
             py::arg("pattern"),
             "Set 2x2 Bayer phase; pattern must be RGGB, GRBG, BGGR, or GBRG (case-insensitive)");
}
