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

// Helper function to convert DngData to numpy array
py::object DngDataToNumpy(const DngData* data) {
    if (!data || !data->ptr) {
        throw std::runtime_error("Invalid DngData pointer");
    }
    
    // Determine element size and type based on pixel type
    size_t element_size = 0;
    bool is_signed = false;
    
    switch (data->pixel_type) {
        case ttByte:
            element_size = sizeof(uint8_t);
            break;
        case ttShort:
            element_size = sizeof(uint16_t);
            break;
        case ttSShort:
            element_size = sizeof(int16_t);
            is_signed = true;
            break;
        case ttLong:
            element_size = sizeof(uint32_t);
            break;
        default:
            element_size = sizeof(uint16_t);
            break;
    }
    
    size_t total_size = data->width * data->height * data->channels * element_size;
    
    // Create numpy array from buffer
    py::capsule free_when_done(data->ptr, [](void *f) {
        free(f);
    });
    
    // Return appropriate numpy array type
    if (element_size == sizeof(uint8_t)) {
        return py::array_t<uint8_t>(
            {data->height, data->width, data->channels},
            {data->width * data->channels * element_size,
             data->channels * element_size,
             element_size},
            static_cast<uint8_t*>(data->ptr),
            free_when_done
        );
    } else if (element_size == sizeof(uint16_t) && !is_signed) {
        return py::array_t<uint16_t>(
            {data->height, data->width, data->channels},
            {data->width * data->channels * element_size,
             data->channels * element_size,
             element_size},
            static_cast<uint16_t*>(data->ptr),
            free_when_done
        );
    } else if (element_size == sizeof(int16_t) && is_signed) {
        return py::array_t<int16_t>(
            {data->height, data->width, data->channels},
            {data->width * data->channels * element_size,
             data->channels * element_size,
             element_size},
            static_cast<int16_t*>(data->ptr),
            free_when_done
        );
    } else {
        // For other types, return as uint8_t array
        return py::array_t<uint8_t>(
            {data->height, data->width, data->channels},
            {data->width * data->channels * element_size,
             data->channels * element_size,
             element_size},
            static_cast<uint8_t*>(data->ptr),
            free_when_done
        );
    }
}

// Helper function to create DngData from numpy array
template<typename T>
DngData* NumpyToDngDataImpl(py::array_t<T> arr, uint32_t pixel_type) {
    auto data = new DngData();
    
    py::buffer_info buf_info = arr.request();
    
    if (buf_info.ndim != 3) {
        delete data;
        throw std::runtime_error("Array must be 3-dimensional (height, width, channels)");
    }
    
    data->height = buf_info.shape[0];
    data->width = buf_info.shape[1];
    data->channels = buf_info.shape[2];
    data->pixel_type = pixel_type;
    data->top = 0;
    data->left = 0;
    
    // Determine element size
    size_t element_size = sizeof(T);
    size_t total_size = data->width * data->height * data->channels * element_size;
    data->ptr = malloc(total_size);
    memcpy(data->ptr, buf_info.ptr, total_size);
    
    return data;
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
    
    // DngData wrapper (we'll use smart pointers to manage memory)
    py::class_<DngData, std::unique_ptr<DngData, py::nodelete>>(m, "DngData")
        .def(py::init<>())
        .def_readonly("width", &DngData::width)
        .def_readonly("height", &DngData::height)
        .def_readonly("channels", &DngData::channels)
        .def_readonly("pixel_type", &DngData::pixel_type)
        .def_readonly("top", &DngData::top)
        .def_readonly("left", &DngData::left)
        .def("to_numpy", [](DngData* data) -> py::object {
            return DngDataToNumpy(data);
        }, "Convert DngData to numpy array")
        .def("__repr__", [](const DngData& data) {
            return "<DngData width=" + std::to_string(data.width) + 
                   " height=" + std::to_string(data.height) + 
                   " channels=" + std::to_string(data.channels) + ">";
        });
    
    // Dng class
    py::class_<Dng>(m, "Dng")
        .def(py::init<>())
        .def(py::init<const std::string&, bool>(),
             py::arg("path"),
             py::arg("ignore_enhanced") = false,
             "Load a DNG from path (same as Dng() then read). Raises RuntimeError on failure.")
        .def("read", &Dng::Read, 
             py::arg("path"), 
             py::arg("ignore_enhanced") = false,
             "Read a DNG file from disk")
        .def("write", &Dng::Write,
             py::arg("path"),
             "Write the DNG file to disk")
        .def("get_data", [](Dng& self, bool enhanced) {
            auto data = self.GetData(enhanced);
            return std::unique_ptr<DngData, py::nodelete>(data);
        }, py::arg("enhanced") = false,
           "Get image data as DngData object")
        .def("set_data", [](Dng& self, py::array arr, uint32_t pixel_type, bool enhanced) {
            DngData* data = nullptr;
            
            // Get dtype
            py::dtype dtype = arr.dtype();
            std::string dtype_str = py::str(dtype);
            
            // Try to convert based on dtype
            if (dtype.is(py::dtype::of<uint8_t>())) {
                data = NumpyToDngDataImpl(py::cast<py::array_t<uint8_t>>(arr), pixel_type);
            } else if (dtype.is(py::dtype::of<uint16_t>())) {
                data = NumpyToDngDataImpl(py::cast<py::array_t<uint16_t>>(arr), pixel_type);
            } else if (dtype.is(py::dtype::of<int16_t>())) {
                data = NumpyToDngDataImpl(py::cast<py::array_t<int16_t>>(arr), pixel_type);
            } else {
                // Try generic conversion
                py::array_t<uint16_t> converted = arr.cast<py::array_t<uint16_t>>();
                data = NumpyToDngDataImpl(converted, pixel_type);
            }
            
            self.SetData(data, enhanced);
            delete data;
        }, py::arg("data"), py::arg("pixel_type"), py::arg("enhanced") = false,
           "Set image data from numpy array")
        .def("get_baseline_exposure", &Dng::GetBaselineExposure,
             "Get baseline exposure value")
        .def("set_baseline_exposure", &Dng::SetBaselineExposure,
             py::arg("exposure"),
             "Set baseline exposure value")
        .def("get_meta", [](Dng& self) {
            auto meta = self.GetMeta();
            return std::unique_ptr<DngMeta>(meta);
        }, "Get metadata from DNG file")
        .def("set_meta", &Dng::SetMeta,
             py::arg("meta"),
             "Set metadata for DNG file")
        .def("get_bayer_pattern", &Dng::GetBayerPattern,
             "Return 2x2 Bayer tile as RGGB/GRBG/BGGR/GBRG, or empty string if not available")
        .def("set_bayer_pattern", &Dng::SetBayerPattern,
             py::arg("pattern"),
             "Set 2x2 Bayer phase; pattern must be RGGB, GRBG, BGGR, or GBRG (case-insensitive)");
}

