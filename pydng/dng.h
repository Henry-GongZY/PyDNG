//
// Created by Henrygongzy on 25-11-16.
//

#ifndef DNG_H
#define DNG_H

#include <dng_host.h>
#include <dng_file_stream.h>
#include <dng_info.h>
#include <dng_negative.h>
#include <dng_image.h>
#include <dng_errors.h>
#include <dng_render.h>
#include <dng_exceptions.h>
#include <dng_image_writer.h>
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
int UTF8ToWChar(const char*, int, wchar_t*, int);
# endif

struct DngData {
    void* ptr;
    uint32 width;
    uint32 height;
    uint32 channels;
    uint32 pixel_type;

    int top, left;

    DngData() : ptr(nullptr), width(0), height(0), channels(0), pixel_type(0), top(0), left(0) {}
};

// DNG 文件信息结构体
struct DngMeta {
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
    DngMeta() : width(0), height(0), rawWidth(0), rawHeight(0),
                    exposureTime(0.0), fNumber(0.0), focalLength(0.0),
                    iso(0), focalLength35mm(0), isMonochrome(false),
                    colorPlanes(0) {}
};

struct Dng {
    AutoPtr<dng_negative> negative;

public:
    int Read(const std::string&, bool);
    int Write(const std::string&);

    DngData *GetImage(bool enhanced) const;

    void SetImage(const DngData *data, bool enhanced) const;
};

// void Dng::SetImage(const cv::Mat &image, bool enhanced) {
//     dng_host host;
//     host.SetPreferredSize(0);
//     host.SetMinimumSize(0);
//     host.SetMaximumSize(0);
//     host.ValidateSizes();
//     if (host.MinimumSize())
//         host.SetForPreview(true);
//
//     // prepare buffer
//     cv::Mat new_image = image.isContinuous() ? image : image.clone();
//     AutoPtr<dng_image> dng_img;
//     height_ = image.rows;
//     width_ = image.cols;
//     auto is_rgb = new_image.channels() == 3;
//     pixelType_ = ToDngType(new_image.depth());
//     dng_img.Reset(host.Make_dng_image(dng_rect(0, 0, height_, width_), new_image.channels(), pixelType_));
//     dng_pixel_buffer buffer(dng_rect(0, 0, height_, width_), 0, new_image.channels(), pixelType_, pcInterleaved, new_image.data);
//     dng_img->Put(buffer);
//
//     if (enhanced) {
//         negative->SetStage3Image(dng_img);
//     }
//     else {
//         negative->SetActiveArea(dng_rect(0, 0, height_, width_));
//         negative->SetStage1Image(dng_img);
//         negative->SetOriginalSizes(dng_point(height_, width_));
//         negative->SetDefaultCropSize(width_, height_);
//         negative->SetDefaultCropOrigin(0, 0);
//         negative->SetRGB();
//         AutoPtr<dng_image> empty_stage3;
//         negative->SetStage3Image(empty_stage3);
//         if (is_rgb) {
//             negative->ClearMosaicInfo();
//             SetBlackLevels({4096, 4096, 4096, 4096});
//             auto white_level = GetWhiteLevel(0);
//             SetWhiteLevel(white_level, 1);
//             SetWhiteLevel(white_level, 2);
//             ClearOpcodeList();
//         }
//     }
// }

// 将 dng_string 转换为 std::string
std::string DNGStringToStdString(const dng_string&);

// 将 dng_urational 转换为 double
double DNGRationalToDouble(const dng_urational&);

// 提取 DNG 文件信息
DngMeta ExtractDNGInfo(AutoPtr<dng_negative>&);

// 打印 DNG 文件信息
void PrintDNGInfo(const DngMeta&);

// 读取 DNG 文件并返回信息
std::pair<dng_error_code, DngMeta> Read(const std::string &path, bool);

#endif //DNG_H
