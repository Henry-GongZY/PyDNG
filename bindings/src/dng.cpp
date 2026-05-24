//
// Created by Henrygongzy on 25-11-16.
//

#include "dng.h"
#include "utils.h"
#include "dng_gain_map.h"
#include <stdexcept>
#include <string>

// Forward declarations of utility functions
std::string DNGStringToStdString(const dng_string& dngStr);
double DNGRationalToDouble(const dng_urational& rational);
char ColorKeyToBayerChar(uint8 key);
uint32_t BayerStringToPhase(const std::string &p);

// ── Lifecycle: constructor ─────────────────────────────────

Dng::Dng(const std::string& path, bool ignore_enhanced) {
    const int err = Read(path, ignore_enhanced);
    if (err != dng_error_none) {
        throw std::runtime_error(
            "Dng: failed to read \"" + path + "\" (error code " + std::to_string(err) + ")");
    }
}

// ── Lifecycle: file I/O ────────────────────────────────────

int Dng::Read(const std::string &path, bool ignore_enhanced) {
    try {
#ifdef WIN32
        wchar_t pathw[1024]{0};
        if (UTF8ToWChar(path.c_str(), 0, pathw, 1024) == 0) {
            std::cout << "Failed to convert to wchar_t:" << path;
            return dng_error_read_file;
        }
        dng_file_stream stream(pathw);
#else
        dng_file_stream stream(path.c_str());
#endif  // WIN32
        dng_host host;
        host.SetPreferredSize(0);
        host.SetMinimumSize(0);
        host.SetMaximumSize(0);
        host.ValidateSizes();
        host.SetSaveDNGVersion(dngVersion_SaveDefault);

        if (host.MinimumSize())
            host.SetForPreview(true);
        dng_info info;
        info.Parse(host, stream);
        info.PostParse(host);
        if (!info.IsValidDNG()) {
            return dng_error_bad_format;
        }
        negative.Reset(host.Make_dng_negative());
        if (!ignore_enhanced && info.fEnhancedIndex != -1) {
            negative->ReadEnhancedImage(host, stream, info);
        }
        if (info.fMaskIndex != -1) {
            negative->ReadTransparencyMask(host, stream, info);
        }
        negative->Parse(host, stream, info);
        negative->PostParse(host, stream, info);
        negative->ReadStage1Image(host, stream, info);
        negative->ValidateRawImageDigest(host);
    }
    catch (...) {
        std::cout << "Error loading DNG file:" << path;
        return dng_error_read_file;
    }

    return dng_error_none;
}

int Dng::Write(const std::string &path) {
    dng_host host;
    dng_image_writer writer;

    host.SetPreferredSize(0);
    host.SetMinimumSize(0);
    host.SetMaximumSize(0);
    host.ValidateSizes();
    if (host.MinimumSize())
        host.SetForPreview(true);

    host.SetSaveDNGVersion(dngVersion_SaveDefault);
    host.SetSaveLinearDNG(false);
    host.SetKeepOriginalFile(false);
    auto &digest = const_cast<dng_fingerprint &>(negative->NewRawImageDigest());
    digest.Clear();
    negative->FindNewRawImageDigest(host);
    negative->ValidateRawImageDigest(host);
#ifdef WIN32
    wchar_t pathw[1024]{0};
    if (UTF8ToWChar(path.c_str(), 0, pathw, 1024) == 0) {
        std::cout << "Failed to convert to wchar_t:" << std::endl;
        return dng_error_write_file;
    }
    dng_file_stream stream(pathw, true);
#else
    dng_file_stream stream(path.c_str(), true);
#endif  // WIN32
    writer.WriteDNG(host, stream, *negative, nullptr, dngVersion_SaveDefault, true);
    return dng_error_none;
}

// ── Image data access ──────────────────────────────────────

DngData* Dng::GetData(bool enhanced) const {
    if (!negative.Get()) {
        throw std::runtime_error("DNG negative is not initialized.");
    }

    const dng_image *dng_image_buf = nullptr;
    if (enhanced) {
        dng_image_buf = negative->Stage3Image();
        if (!dng_image_buf) {
            throw std::runtime_error("Stage 3 (enhanced) image data is not available in this DNG.");
        }
    } else {
        dng_image_buf = negative->Stage1Image();
        if (!dng_image_buf) {
            throw std::runtime_error("Stage 1 (raw) image data is not available in this DNG.");
        }
    }

    auto data = new DngData();
    data->width = dng_image_buf->Width();
    data->height = dng_image_buf->Height();
    data->pixel_type = dng_image_buf->PixelType();
    data->channels = dng_image_buf->Planes();

    int bytes_per_pixel = 0;
    switch (data->pixel_type) {
        case ttByte:   bytes_per_pixel = 1; break;
        case ttShort:  bytes_per_pixel = 2; break;
        case ttSShort: bytes_per_pixel = 2; break;
        case ttLong:   bytes_per_pixel = 4; break;
        case ttFloat:  bytes_per_pixel = 4; break;
        default:       bytes_per_pixel = 2; break;
    }

    data->ptr = malloc(data->width * data->height * data->channels * bytes_per_pixel);
    if (!data->ptr) {
        delete data;
        throw std::runtime_error("Failed to allocate memory for image data.");
    }

    dng_pixel_buffer buffer(dng_rect(0, 0, static_cast<int32>(data->height), static_cast<int32>(data->width)),
                            0, data->channels, data->pixel_type, pcInterleaved, data->ptr);

    dng_image_buf->Get(buffer, dng_image::edge_none);

    // Default active area
    data->top = 0;
    data->left = 0;

    // Try to get linearization active area if available
    if (negative->GetLinearizationInfo()) {
        auto active_area = negative->GetLinearizationInfo()->fActiveArea;
        data->top = active_area.t;
        data->left = active_area.l;
    }

    return data;
}

void Dng::SetData(const DngData* data, bool enhanced) const {
    dng_host host;
    host.SetPreferredSize(0);
    host.SetMinimumSize(0);
    host.SetMaximumSize(0);
    host.ValidateSizes();
    if (host.MinimumSize())
        host.SetForPreview(true);

    AutoPtr<dng_image> dng_img;
    dng_img.Reset(host.Make_dng_image(dng_rect(0, 0, static_cast<int32>(data->height), static_cast<int32>(data->width)), data->channels, data->pixel_type));
    dng_pixel_buffer buffer(dng_rect(0, 0, static_cast<int32>(data->height), static_cast<int32>(data->width)), 0, data->channels, data->pixel_type, pcInterleaved, data->ptr);
    dng_img->Put(buffer);

    if (enhanced) {
        negative->SetStage3Image(dng_img);
    }
    else {
        negative->SetActiveArea(dng_rect(0, 0, static_cast<int32>(data->height), static_cast<int32>(data->width)));
        negative->SetStage1Image(dng_img);
        negative->SetOriginalSizes(dng_point(static_cast<int32>(data->height), static_cast<int32>(data->width)));
        negative->SetDefaultCropSize(static_cast<int32>(data->width), static_cast<int32>(data->height));
        negative->SetDefaultCropOrigin(0, 0);
        negative->SetRGB();
        AutoPtr<dng_image> empty_stage3;
        negative->SetStage3Image(empty_stage3);
        if (data->channels == 3) {
            negative->ClearMosaicInfo();
        }
    }
}

// ── Metadata ───────────────────────────────────────────────

DngMeta* Dng::GetMeta() const {
    auto meta = new DngMeta();
    if (!negative.Get()) {
        return meta;
    }

    DngMeta* exif = GetExif();
    DngMeta* img = GetImageInfo();
    DngMeta* color = GetColorInfo();

    // Merge metadata
    meta->make = exif->make;
    meta->model = exif->model;
    meta->software = exif->software;
    meta->artist = exif->artist;
    meta->copyright = exif->copyright;
    meta->exposureTime = exif->exposureTime;
    meta->fNumber = exif->fNumber;
    meta->focalLength = exif->focalLength;
    meta->iso = exif->iso;
    meta->focalLength35mm = exif->focalLength35mm;
    meta->dateTime = exif->dateTime;
    meta->dateTimeOriginal = exif->dateTimeOriginal;

    meta->width = img->width;
    meta->height = img->height;
    meta->rawWidth = img->rawWidth;
    meta->rawHeight = img->rawHeight;

    meta->isMonochrome = color->isMonochrome;
    meta->colorPlanes = color->colorPlanes;
    meta->colorSpace = color->colorSpace;

    delete exif;
    delete img;
    delete color;

    return meta;
}

DngMeta* Dng::GetExif() const {
    auto meta = new DngMeta();
    if (!negative.Get()) return meta;

    // Synchronize metadata
    negative->SynchronizeMetadata();

    const dng_exif* exif = negative->GetExif();
    if (exif) {
        // Basic info
        meta->make = DNGStringToStdString(exif->fMake);
        meta->model = DNGStringToStdString(exif->fModel);
        meta->software = DNGStringToStdString(exif->fSoftware);
        meta->artist = DNGStringToStdString(exif->fArtist);
        meta->copyright = DNGStringToStdString(exif->fCopyright);

        // Camera settings
        meta->exposureTime = DNGRationalToDouble(exif->fExposureTime);
        meta->fNumber = DNGRationalToDouble(exif->fFNumber);
        meta->focalLength = DNGRationalToDouble(exif->fFocalLength);
        meta->focalLength35mm = exif->fFocalLengthIn35mmFilm;

        // ISO sensitivity
        if (exif->fISOSpeed != 0) {
            meta->iso = exif->fISOSpeed;
        } else if (exif->fISOSpeedRatings[0] != 0) {
            meta->iso = exif->fISOSpeedRatings[0];
        } else if (exif->fStandardOutputSensitivity != 0) {
            meta->iso = exif->fStandardOutputSensitivity;
        }

        // Date/time
        if (exif->fDateTime.IsValid()) {
            dng_string dateTimeStr = exif->fDateTime.Encode_ISO_8601();
            meta->dateTime = DNGStringToStdString(dateTimeStr);
        }
        if (exif->fDateTimeOriginal.IsValid()) {
            dng_string dateTimeStr = exif->fDateTimeOriginal.Encode_ISO_8601();
            meta->dateTimeOriginal = DNGStringToStdString(dateTimeStr);
        }
    }
    return meta;
}

DngMeta* Dng::GetImageInfo() const {
    auto meta = new DngMeta();
    if (!negative.Get()) return meta;

    // Image dimensions
    if (negative->Stage1Image()) {
        dng_point stage1Size = negative->Stage1Image()->Size();
        meta->rawWidth = stage1Size.h;
        meta->rawHeight = stage1Size.v;
    }

    // Get default crop size (actual image dimensions)
    dng_urational cropSizeH = negative->DefaultCropSizeH();
    dng_urational cropSizeV = negative->DefaultCropSizeV();
    meta->width = static_cast<uint32_t>(cropSizeH.As_real64());
    meta->height = static_cast<uint32_t>(cropSizeV.As_real64());

    // Fall back to raw size if crop is 0
    if (meta->width == 0 || meta->height == 0) {
        meta->width = meta->rawWidth;
        meta->height = meta->rawHeight;
    }
    return meta;
}

DngMeta* Dng::GetColorInfo() const {
    auto meta = new DngMeta();
    if (!negative.Get()) return meta;

    meta->isMonochrome = negative->IsMonochrome();
    if (negative->Stage1Image()) {
        meta->colorPlanes = negative->Stage1Image()->Planes();
    }

    // Color space
    if (negative->IsMonochrome()) {
        meta->colorSpace = "Grayscale";
    } else {
        meta->colorSpace = "RGB";
    }
    return meta;
}

void Dng::SetMeta(const DngMeta* /*meta*/) {

}

// ── Exposure & white balance ───────────────────────────────

double Dng::GetBaselineExposure() const {
    return negative->BaselineExposure();
}

void Dng::SetBaselineExposure(double exposure) {
    negative->SetBaselineExposure(exposure);
}

std::vector<double> Dng::GetWhiteBalance() const {
    std::vector<double> wb;
    if (!negative.Get()) return wb;

    if (negative->HasCameraNeutral()) {
        const dng_vector& neutral = negative->CameraNeutral();
        wb.reserve(neutral.Count());
        for (uint32 i = 0; i < neutral.Count(); ++i) {
            wb.push_back(neutral[i]);
        }
    }
    return wb;
}

void Dng::SetWhiteBalance(const std::vector<double>& wb) {
    if (!negative.Get() || wb.empty()) return;

    dng_vector neutral((uint32)wb.size());
    for (uint32 i = 0; i < (uint32)wb.size(); ++i) {
        neutral[i] = wb[i];
    }
    negative->SetCameraNeutral(neutral);
}

// ── Gain map ────────────────────────────────────────────────

DngGainMap* Dng::GetGainmap() const {
    if (!negative.Get()) return nullptr;

    const dng_opcode_list& list = negative->OpcodeList2();
    for (uint32 i = 0; i < list.Count(); ++i) {
        const dng_opcode& op = list.Entry(i);
        if (op.OpcodeID() == dngOpcode_GainMap) {
            const auto& gainOp = static_cast<const dng_opcode_GainMap&>(op);
            const dng_gain_map& map = gainOp.GainMap();

            auto* result = new DngGainMap();
            result->rows = map.Points().v;
            result->cols = map.Points().h;
            result->planes = map.Planes();
            result->spacingV = map.Spacing().v;
            result->spacingH = map.Spacing().h;
            result->originV = map.Origin().v;
            result->originH = map.Origin().h;

            result->data.resize(result->rows * result->cols * result->planes);
            for (uint32 r = 0; r < result->rows; ++r) {
                for (uint32 c = 0; c < result->cols; ++c) {
                    for (uint32 p = 0; p < result->planes; ++p) {
                        result->data[r * result->cols * result->planes + c * result->planes + p] = map.Entry(r, c, p);
                    }
                }
            }
            return result;
        }
    }
    return nullptr;
}

void Dng::SetGainmap(const DngGainMap* map) {
    if (!negative.Get() || !map) return;

    dng_point points(map->rows, map->cols);
    dng_point_real64 spacing(map->spacingV, map->spacingH);
    dng_point_real64 origin(map->originV, map->originH);

    AutoPtr<dng_gain_map> dngMap(new dng_gain_map(negative->Allocator(), points, spacing, origin, map->planes));

    for (uint32 r = 0; r < map->rows; ++r) {
        for (uint32 c = 0; c < map->cols; ++c) {
            for (uint32 p = 0; p < map->planes; ++p) {
                dngMap->Entry(r, c, p) = map->data[r * map->cols * map->planes + c * map->planes + p];
            }
        }
    }

    // Default to full active area
    dng_rect area = negative->Stage1Image() ? negative->Stage1Image()->Bounds() : dng_rect(0, 0, 0, 0);
    if (area.IsEmpty()) {
        // Fallback to default crop or whatever
    }

    dng_area_spec areaSpec;
    areaSpec = dng_area_spec(area, 0, map->planes, 1, 1);

    AutoPtr<dng_opcode> opcode(new dng_opcode_GainMap(areaSpec, dngMap));

    dng_opcode_list& list = negative->OpcodeList2();
    for (int32 i = (int32)list.Count() - 1; i >= 0; --i) {
        if (list.Entry(i).OpcodeID() == dngOpcode_GainMap) {
            list.Remove(i);
        }
    }
    list.Append(opcode);
}

// ── Bayer pattern ──────────────────────────────────────────

std::string Dng::GetBayerPattern() const {
    if (!negative.Get()) return "";
    const dng_mosaic_info *mosaic = negative->GetMosaicInfo();
    if (!mosaic || !mosaic->IsColorFilterArray()) return "";
    if (mosaic->fCFAPatternSize.v != 2 || mosaic->fCFAPatternSize.h != 2) return "";
    if (mosaic->fCFALayout != 1) return "";

    std::string out;
    out.reserve(4);
    for (int32 j = 0; j < 2; ++j) {
        for (int32 k = 0; k < 2; ++k) {
            const uint8 planeIdx = mosaic->fCFAPattern[j][k];
            if (planeIdx >= mosaic->fColorPlanes) return "";
            const uint8 key = mosaic->fCFAPlaneColor[planeIdx];
            const char ch = ColorKeyToBayerChar(key);
            if (ch == 0) return "";
            out += ch;
        }
    }
    return out;
}

void Dng::SetBayerPattern(const std::string &pattern) {
    if (!negative.Get()) {
        throw std::runtime_error("Dng::SetBayerPattern: no image loaded");
    }
    const uint32_t phase = BayerStringToPhase(pattern);
    if (phase == static_cast<uint32_t>(-1)) {
        throw std::invalid_argument(
            "SetBayerPattern: expected one of RGGB, GRBG, BGGR, GBRG (2x2 Bayer, R/G/B only)");
    }

    // NeedMosaicInfo() is protected on dng_negative; SetRGB() / SetBayerMosaic() call it internally.
    const dng_mosaic_info *mosaic = negative->GetMosaicInfo();
    if (!mosaic || mosaic->fColorPlanes == 0) {
        negative->SetRGB();
    } else if (mosaic->fColorPlanes != 3) {
        throw std::invalid_argument(
            "SetBayerPattern: requires a 3-plane RGB CFA (or empty mosaic); use a file with standard Bayer metadata");
    }

    negative->SetBayerMosaic(phase);
}
