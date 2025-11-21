//
// Created by Henrygongzy on 25-11-16.
//

#include "dng.h"
#include "utils.h"

// 前向声明工具函数
std::string DNGStringToStdString(const dng_string& dngStr);
double DNGRationalToDouble(const dng_urational& rational);

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

DngData* Dng::GetData(bool enhanced) const {
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
            break;
        case ttSShort:
            size = sizeof(short);
            break;
        case ttByte:
            size = sizeof(unsigned char);
            break;
        case ttLong:
            size = sizeof(long);
            break;
        default:
            size = sizeof(unsigned short);
            break;
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

double Dng::GetBaselineExposure() const {
    return negative->BaselineExposure();
}

void Dng::SetBaselineExposure(double exposure) {
    negative->SetBaselineExposure(exposure);
}

DngMeta* Dng::GetMeta() const {
    if (!negative.Get()) {
        return new DngMeta();
    }
    
    auto meta = new DngMeta();
    
    // 同步元数据
    negative->SynchronizeMetadata();
    
    // 获取 EXIF 信息
    const dng_exif* exif = negative->GetExif();
    if (exif) {
        // 基本信息
        meta->make = DNGStringToStdString(exif->fMake);
        meta->model = DNGStringToStdString(exif->fModel);
        meta->software = DNGStringToStdString(exif->fSoftware);
        meta->artist = DNGStringToStdString(exif->fArtist);
        meta->copyright = DNGStringToStdString(exif->fCopyright);
        
        // 拍摄参数
        meta->exposureTime = DNGRationalToDouble(exif->fExposureTime);
        meta->fNumber = DNGRationalToDouble(exif->fFNumber);
        meta->focalLength = DNGRationalToDouble(exif->fFocalLength);
        meta->focalLength35mm = exif->fFocalLengthIn35mmFilm;
        
        // ISO 感光度
        if (exif->fISOSpeed != 0) {
            meta->iso = exif->fISOSpeed;
        } else if (exif->fISOSpeedRatings[0] != 0) {
            meta->iso = exif->fISOSpeedRatings[0];
        } else if (exif->fStandardOutputSensitivity != 0) {
            meta->iso = exif->fStandardOutputSensitivity;
        }
        
        // 日期时间
        if (exif->fDateTime.IsValid()) {
            dng_string dateTimeStr = exif->fDateTime.Encode_ISO_8601();
            meta->dateTime = DNGStringToStdString(dateTimeStr);
        }
        if (exif->fDateTimeOriginal.IsValid()) {
            dng_string dateTimeStr = exif->fDateTimeOriginal.Encode_ISO_8601();
            meta->dateTimeOriginal = DNGStringToStdString(dateTimeStr);
        }
    }
    
    // 图像尺寸
    if (negative->Stage1Image()) {
        dng_point stage1Size = negative->Stage1Image()->Size();
        meta->rawWidth = stage1Size.h;
        meta->rawHeight = stage1Size.v;
    }
    
    // 获取默认裁剪尺寸（实际图像尺寸）
    dng_urational cropSizeH = negative->DefaultCropSizeH();
    dng_urational cropSizeV = negative->DefaultCropSizeV();
    meta->width = static_cast<uint32_t>(cropSizeH.As_real64());
    meta->height = static_cast<uint32_t>(cropSizeV.As_real64());
    
    // 如果裁剪尺寸为0，使用原始尺寸
    if (meta->width == 0 || meta->height == 0) {
        meta->width = meta->rawWidth;
        meta->height = meta->rawHeight;
    }
    
    // 其他信息
    meta->isMonochrome = negative->IsMonochrome();
    if (negative->Stage1Image()) {
        meta->colorPlanes = negative->Stage1Image()->Planes();
    }
    
    // 色彩空间
    if (negative->IsMonochrome()) {
        meta->colorSpace = "Grayscale";
    } else {
        meta->colorSpace = "RGB";
    }
    
    return meta;
}

void Dng::SetMeta(const DngMeta* meta) {

}
