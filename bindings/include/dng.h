//
// Created by Henrygongzy on 25-11-16.
//

#ifndef DNG_H
#define DNG_H

#include <string>

#include "pch.h"

// DNG 图像数据
struct DngData {
    void* ptr;
    uint32 width;
    uint32 height;
    uint32 channels;
    uint32 pixel_type;

    int top, left;

    DngData() : ptr(nullptr), width(0), height(0), channels(0), pixel_type(0), top(0), left(0) {}
};

// DNG 文件信息
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
    Dng() = default;
    /// Load a DNG from disk (equivalent to default constructor then Read). Throws on failure.
    explicit Dng(const std::string& path, bool ignore_enhanced = false);

    int Read(const std::string&, bool);
    int Write(const std::string&);

    DngData* GetData(bool enhanced) const;

    void SetData(const DngData* data, bool enhanced) const;

    double GetBaselineExposure() const;

    void SetBaselineExposure(double exposure);

    DngMeta *GetMeta() const;
    DngMeta *GetExif() const;
    DngMeta *GetImageInfo() const;
    DngMeta *GetColorInfo() const;
    std::vector<double> GetWhiteBalance() const;
    void SetWhiteBalance(const std::vector<double> &wb);

    void SetMeta(const DngMeta *meta);

    /// Row-major 2×2 Bayer tile as "RGGB" / "GRBG" / "BGGR" / "GBRG", or empty if unavailable / not 2×2 RGB CFA.
    std::string GetBayerPattern() const;

    /// Set standard 2×2 Bayer phase from the same four strings (case-insensitive). Requires RGB CFA semantics; may call SetRGB() when mosaic planes are not yet set.
    void SetBayerPattern(const std::string &pattern);
};


#endif //DNG_H

