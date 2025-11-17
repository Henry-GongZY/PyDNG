//
// Created by Henrygongzy on 25-11-16.
//

#include "dng.h"

# ifdef WIN32
int UTF8ToWChar(const char* utf8Str, int utf8Len, wchar_t* wStr, int wStrLen) {
    if (utf8Str == nullptr || wStr == nullptr || wStrLen <= 0) {
        return 0;
    }
    // 若 utf8Len 为 0，自动计算字符串长度（不含 '\0'）
    int actualUtf8Len = (utf8Len == 0) ? lstrlenA(utf8Str) : utf8Len;
    // 调用 Windows 系统 API MultiByteToWideChar，指定 UTF-8 代码页
    return MultiByteToWideChar(
        CP_UTF8,          // 源编码：UTF-8
        0,                // 转换标志（0 为默认）
        utf8Str,          // 源 UTF-8 字符串
        actualUtf8Len,    // 源字符串长度（-1 表示包含 '\0' 自动终止）
        wStr,             // 目标宽字符串缓冲区
        wStrLen           // 目标缓冲区最大长度（单位：wchar_t）
    );
}
# endif

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

DngData* Dng::GetImage(bool enhanced) const {
    const dng_image *dng_image_buf = nullptr;
    auto data = new DngData();
    if (enhanced)
        dng_image_buf = negative->Stage3Image();
    else
        dng_image_buf = negative->Stage1Image();

    data->width = dng_image_buf->Width();
    data->height = dng_image_buf->Height();
    data->pixel_type = dng_image_buf->PixelType();
    data->channels = dng_image_buf->Planes();
    int size;
    switch (data->pixel_type) {
        case ttShort:
            size = sizeof(unsigned short);
        case ttSShort:
            size = sizeof(short);
        case ttByte:
            size = sizeof(unsigned char);
        case ttLong:
            size = sizeof(long);
        default:
            size = sizeof(unsigned short);
    }
    data->ptr = malloc(data->width * data->height * data->channels * size);

    dng_pixel_buffer buffer(dng_rect(0, 0, static_cast<int32>(data->height), static_cast<int32>(data->width)),
                                0, data->channels, data->pixel_type, pcInterleaved, data->ptr);
    dng_image_buf->Get(buffer, dng_image::edge_none);

    auto active_area = negative->GetLinearizationInfo()->fActiveArea;
    data->top = active_area.t;
    data->left = active_area.l;

    return data;
}

void Dng::SetImage(const DngData* data, bool enhanced) const {
    dng_host host;
    host.SetPreferredSize(0);
    host.SetMinimumSize(0);
    host.SetMaximumSize(0);
    host.ValidateSizes();
    if (host.MinimumSize())
        host.SetForPreview(true);

    // prepare buffer
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

// 将 dng_string 转换为 std::string
std::string DNGStringToStdString(const dng_string& dngStr) {
    if (dngStr.IsEmpty()) {
        return "";
    }
    return std::string(dngStr.Get());
}

// 将 dng_urational 转换为 double
double DNGRationalToDouble(const dng_urational& rational) {
    if (rational.d == 0) {
        return 0.0;
    }
    return static_cast<double>(rational.n) / static_cast<double>(rational.d);
}

// 提取 DNG 文件信息
DngMeta ExtractDNGInfo(AutoPtr<dng_negative>& negative) {
    DngMeta info;

    if (!negative.Get()) {
        return info;
    }

    // 同步元数据
    negative->SynchronizeMetadata();

    // 获取 EXIF 信息
    const dng_exif* exif = negative->GetExif();
    if (!exif) {
        return info;
    }

    // 基本信息
    info.make = DNGStringToStdString(exif->fMake);
    info.model = DNGStringToStdString(exif->fModel);
    info.software = DNGStringToStdString(exif->fSoftware);
    info.artist = DNGStringToStdString(exif->fArtist);
    info.copyright = DNGStringToStdString(exif->fCopyright);

    // 图像尺寸
    if (negative->Stage1Image()) {
        dng_point stage1Size = negative->Stage1Image()->Size();
        info.rawWidth = stage1Size.h;
        info.rawHeight = stage1Size.v;
    }

    // 获取默认裁剪尺寸（实际图像尺寸）
    dng_urational cropSizeH = negative->DefaultCropSizeH();
    dng_urational cropSizeV = negative->DefaultCropSizeV();
    info.width = static_cast<uint32_t>(cropSizeH.As_real64());
    info.height = static_cast<uint32_t>(cropSizeV.As_real64());

    // 如果裁剪尺寸为0，使用原始尺寸
    if (info.width == 0 || info.height == 0) {
        info.width = info.rawWidth;
        info.height = info.rawHeight;
    }

    // 拍摄参数
    info.exposureTime = DNGRationalToDouble(exif->fExposureTime);
    info.fNumber = DNGRationalToDouble(exif->fFNumber);
    info.focalLength = DNGRationalToDouble(exif->fFocalLength);
    info.focalLength35mm = exif->fFocalLengthIn35mmFilm;

    // ISO 感光度
    if (exif->fISOSpeed != 0) {
        info.iso = exif->fISOSpeed;
    } else if (exif->fISOSpeedRatings[0] != 0) {
        info.iso = exif->fISOSpeedRatings[0];
    } else if (exif->fStandardOutputSensitivity != 0) {
        info.iso = exif->fStandardOutputSensitivity;
    }

    // 日期时间
    if (exif->fDateTime.IsValid()) {
        dng_string dateTimeStr = exif->fDateTime.Encode_ISO_8601();
        info.dateTime = DNGStringToStdString(dateTimeStr);
    }
    if (exif->fDateTimeOriginal.IsValid()) {
        dng_string dateTimeStr = exif->fDateTimeOriginal.Encode_ISO_8601();
        info.dateTimeOriginal = DNGStringToStdString(dateTimeStr);
    }

    // 其他信息
    info.isMonochrome = negative->IsMonochrome();
    if (negative->Stage1Image()) {
        info.colorPlanes = negative->Stage1Image()->Planes();
    }

    // 色彩空间
    if (negative->IsMonochrome()) {
        info.colorSpace = "Grayscale";
    } else {
        info.colorSpace = "RGB";
    }

    return info;
}

// 打印 DNG 文件信息
void PrintDNGInfo(const DngMeta& info) {
    std::cout << "=== DNG File Information ===" << std::endl;
    std::cout << "Make: " << (info.make.empty() ? "Unknown" : info.make) << std::endl;
    std::cout << "Model: " << (info.model.empty() ? "Unknown" : info.model) << std::endl;
    std::cout << "Software: " << (info.software.empty() ? "Unknown" : info.software) << std::endl;
    std::cout << "Artist: " << (info.artist.empty() ? "Unknown" : info.artist) << std::endl;
    std::cout << "Copyright: " << (info.copyright.empty() ? "Unknown" : info.copyright) << std::endl;
    std::cout << std::endl;

    std::cout << "Image Size: " << info.width << " x " << info.height << std::endl;
    std::cout << "RAW Size: " << info.rawWidth << " x " << info.rawHeight << std::endl;
    std::cout << std::endl;

    std::cout << "Camera Settings:" << std::endl;
    std::cout << "  Exposure Time: " << (info.exposureTime > 0 ? std::to_string(info.exposureTime) + " sec" : "Unknown") << std::endl;
    std::cout << "  F-Number: " << (info.fNumber > 0 ? "f/" + std::to_string(info.fNumber) : "Unknown") << std::endl;
    std::cout << "  Focal Length: " << (info.focalLength > 0 ? std::to_string(info.focalLength) + " mm" : "Unknown") << std::endl;
    if (info.focalLength35mm > 0) {
        std::cout << "  35mm Equivalent: " << info.focalLength35mm << " mm" << std::endl;
    }
    std::cout << "  ISO: " << (info.iso > 0 ? std::to_string(info.iso) : "Unknown") << std::endl;
    std::cout << std::endl;

    std::cout << "Date/Time:" << std::endl;
    std::cout << "  DateTime: " << (info.dateTime.empty() ? "Unknown" : info.dateTime) << std::endl;
    std::cout << "  DateTimeOriginal: " << (info.dateTimeOriginal.empty() ? "Unknown" : info.dateTimeOriginal) << std::endl;
    std::cout << std::endl;

    std::cout << "Other Information:" << std::endl;
    std::cout << "  Monochrome: " << (info.isMonochrome ? "Yes" : "No") << std::endl;
    std::cout << "  Color Planes: " << info.colorPlanes << std::endl;
    std::cout << "  Color Space: " << info.colorSpace << std::endl;
    std::cout << "===================" << std::endl;
}

// 读取 DNG 文件并返回信息
std::pair<dng_error_code, DngMeta> Read(const std::string &path, bool ignore_enhanced) {
    DngMeta fileInfo;
    AutoPtr<dng_negative> negative;

    try {
#ifdef WIN32
        wchar_t pathw[1024]{0};
        if (UTF8ToWChar(path.c_str(), 0, pathw, 1024) == 0) {
            std::cout << "Error: Failed to convert path to wide character: " << path << std::endl;
            return std::make_pair(static_cast<dng_error_code>(dng_error_read_file), fileInfo);
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
        host.SetIgnoreEnhanced(ignore_enhanced);

        if (host.MinimumSize())
            host.SetForPreview(true);

        dng_info info;
        info.Parse(host, stream);
        info.PostParse(host);

        if (!info.IsValidDNG()) {
            std::cout << "Error: Not a valid DNG file" << std::endl;
            return std::make_pair(static_cast<dng_error_code>(dng_error_bad_format), fileInfo);
        }

        negative.Reset(host.Make_dng_negative());

        // 读取增强图像（如果存在且未忽略）
        if (!ignore_enhanced && info.fEnhancedIndex != -1) {
            negative->ReadEnhancedImage(host, stream, info);
        }

        // 读取透明度遮罩（如果存在）
        if (info.fMaskIndex != -1) {
            negative->ReadTransparencyMask(host, stream, info);
        }

        // 解析负片信息
        negative->Parse(host, stream, info);
        negative->PostParse(host, stream, info);

        // 读取 Stage1 图像
        negative->ReadStage1Image(host, stream, info);

        // 验证原始图像摘要
        negative->ValidateRawImageDigest(host);

        // 提取文件信息
        fileInfo = ExtractDNGInfo(negative);

        return std::make_pair(dng_error_none, fileInfo);
    }
    catch (const dng_exception& except) {
        std::cout << "Error: DNG exception - " << except.ErrorCode() << std::endl;
        return std::make_pair(except.ErrorCode(), fileInfo);
    }
    catch (...) {
        std::cout << "Error: Unknown exception while loading DNG file: " << path << std::endl;
        return std::make_pair(static_cast<dng_error_code>(dng_error_unknown), fileInfo);
    }
}
