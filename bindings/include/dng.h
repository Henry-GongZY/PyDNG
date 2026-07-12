//
// Created by Henrygongzy on 25-11-16.
//

#ifndef DNG_H
#define DNG_H

#include <cstdint>
#include <string>
#include <vector>

#include "pch.h"

// DNG image data
struct DngData {
    uint32 width;
    uint32 height;
    uint32 channels;
    uint32 pixel_type;
    int top, left;
    std::vector<std::uint8_t> bytes;

    DngData() : width(0), height(0), channels(0), pixel_type(0), top(0), left(0) {}

    void* Data() { return bytes.empty() ? nullptr : bytes.data(); }
    const void* Data() const { return bytes.empty() ? nullptr : bytes.data(); }
};

// DNG metadata
struct DngMeta {
    // Basic info
    std::string make;              // Camera make
    std::string model;             // Camera model
    std::string software;          // Software
    std::string artist;            // Artist
    std::string copyright;         // Copyright

    // Image dimensions
    uint32_t width;                // Image width
    uint32_t height;               // Image height
    uint32_t rawWidth;             // Raw image width
    uint32_t rawHeight;            // Raw image height

    // Camera settings
    double exposureTime;           // Exposure time (sec)
    double fNumber;                // Aperture f-number
    double focalLength;            // Focal length (mm)
    uint32_t iso;                  // ISO sensitivity
    uint32_t focalLength35mm;      // 35mm equivalent focal length

    // Date/time
    std::string dateTime;           // Capture date/time
    std::string dateTimeOriginal;   // Original capture date/time

    // Other info
    bool isMonochrome;             // Monochrome flag
    uint32_t colorPlanes;          // Color plane count
    std::string colorSpace;        // Color space

    // Constructor
    DngMeta() : width(0), height(0), rawWidth(0), rawHeight(0),
                    exposureTime(0.0), fNumber(0.0), focalLength(0.0),
                    iso(0), focalLength35mm(0), isMonochrome(false),
                    colorPlanes(0) {}
};


struct DngGainMap {
    std::vector<float> data;
    uint32_t rows = 0;
    uint32_t cols = 0;
    uint32_t planes = 0;
    double spacingV = 0.0;
    double spacingH = 0.0;
    double originV = 0.0;
    double originH = 0.0;
};

struct Dng {
    AutoPtr<dng_negative> negative;

public:
    Dng() = default;
    /// Load a DNG from disk (equivalent to default constructor then Read). Throws on failure.
    Dng(const std::string& path, bool ignore_enhanced = false);

    // ── Lifecycle: file I/O ─────────────────────────────────
    int Read(const std::string&, bool);
    int Write(const std::string&);

    // ── Image data access ───────────────────────────────────
    DngData GetData(bool enhanced) const;
    DngData GetDataInfo(bool enhanced) const;
    void SetData(const DngData& data, bool enhanced);

    // ── Metadata ────────────────────────────────────────────
    DngMeta GetMeta() const;
    DngMeta GetExif() const;
    DngMeta GetImageInfo() const;
    DngMeta GetColorInfo() const;
    void SetMeta(const DngMeta *meta);

    // ── Exposure & white balance ────────────────────────────
    double GetBaselineExposure() const;
    void SetBaselineExposure(double exposure);
    std::vector<double> GetWhiteBalance() const;
    void SetWhiteBalance(const std::vector<double> &wb);

    // ── Gain map ────────────────────────────────────────────
    DngGainMap *GetGainmap() const;
    void SetGainmap(const DngGainMap *map);

    // ── Bayer pattern ───────────────────────────────────────
    /// Row-major 2×2 Bayer tile as "RGGB" / "GRBG" / "BGGR" / "GBRG", or empty if unavailable / not 2×2 RGB CFA.
    std::string GetBayerPattern() const;
    /// Set standard 2×2 Bayer phase from the same four strings (case-insensitive). Requires RGB CFA semantics; may call SetRGB() when mosaic planes are not yet set.
    void SetBayerPattern(const std::string &pattern);
};


#endif //DNG_H
