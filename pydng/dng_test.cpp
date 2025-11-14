//
// Created by Henrygongzy on 25-11-9.
//

#include <dng_host.h>
#include <dng_file_stream.h>
#include <dng_info.h>
#include <dng_negative.h>
#include <dng_image.h>
#include <dng_errors.h>
#include <dng_render.h>
#include <dng_exceptions.h>
#include <dng_color_space.h>
#include <dng_exif.h>
#include <dng_date_time.h>

#include <iostream>
#include <string>
#include <utility>

#ifdef WIN32
#include <windows.h>
#endif

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

// DNG 文件信息结构体
struct DNGFileInfo {
    // 基本信息
    std::string make;              // 相机厂商
    std::string model;             // 相机型号
    std::string software;          // 软件信息
    std::string artist;            // 艺术家
    std::string copyright;         // 版权信息
    
    // 图像尺寸
    uint32_t width;                // 图像宽度
    uint32_t height;               // 图像高度
    uint32_t rawWidth;             // RAW 图像宽度
    uint32_t rawHeight;            // RAW 图像高度
    
    // 拍摄参数
    double exposureTime;           // 曝光时间（秒）
    double fNumber;                // 光圈值
    double focalLength;            // 焦距（mm）
    uint32_t iso;                  // ISO 感光度
    uint32_t focalLength35mm;      // 35mm 等效焦距
    
    // 日期时间
    std::string dateTime;           // 拍摄日期时间
    std::string dateTimeOriginal;   // 原始拍摄日期时间
    
    // 其他信息
    bool isMonochrome;             // 是否为单色图像
    uint32_t colorPlanes;          // 颜色平面数
    std::string colorSpace;        // 色彩空间
    
    // 构造函数
    DNGFileInfo() : width(0), height(0), rawWidth(0), rawHeight(0),
                    exposureTime(0.0), fNumber(0.0), focalLength(0.0),
                    iso(0), focalLength35mm(0), isMonochrome(false),
                    colorPlanes(0) {}
};

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
DNGFileInfo ExtractDNGInfo(AutoPtr<dng_negative>& negative) {
    DNGFileInfo info;
    
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
void PrintDNGInfo(const DNGFileInfo& info) {
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
std::pair<dng_error_code, DNGFileInfo> Read(const std::string &path, bool ignore_enhanced) {
    DNGFileInfo fileInfo;
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

int main() {
    std::string filePath = R"(C:\Users\Henrygongzy\Desktop\Projects\OpenSource\PyDNG\extern\sample_files\01_jxl_linear_raw_integer.dng)";
    
    auto result = Read(filePath, true);
    
    if (result.first == dng_error_none) {
        PrintDNGInfo(result.second);
    } else {
        std::cout << "Failed to read DNG file, error code: " << result.first << std::endl;
    }
    
    return 0;
}

