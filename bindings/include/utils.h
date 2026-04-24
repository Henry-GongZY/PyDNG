//
// Created by Henrygongzy on 25-11-22.
//

#ifndef UTILS_H
#define UTILS_H

#include "pch.h"


# ifdef WIN32
inline int UTF8ToWChar(const char* utf8Str, int utf8Len, wchar_t* wStr, int wStrLen) {
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

// 将 dng_string 转换为 std::string
inline std::string DNGStringToStdString(const dng_string& dngStr) {
    if (dngStr.IsEmpty()) {
        return "";
    }
    return std::string(dngStr.Get());
}

// 将 dng_urational 转换为 double
inline double DNGRationalToDouble(const dng_urational& rational) {
    if (rational.d == 0) {
        return 0.0;
    }
    return static_cast<double>(rational.n) / static_cast<double>(rational.d);
}

inline char ColorKeyToBayerChar(uint8 key) {
    if (key == colorKeyRed) return 'R';
    if (key == colorKeyGreen) return 'G';
    if (key == colorKeyBlue) return 'B';
    return 0;
}

inline uint32_t BayerStringToPhase(const std::string &p) {
    if (p.size() != 4) return static_cast<uint32_t>(-1);
    std::string u;
    u.reserve(4);
    for (char c : p) {
        if (!std::isalpha(static_cast<unsigned char>(c))) return static_cast<uint32_t>(-1);
        u.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    if (u == "RGGB") return 1;
    if (u == "GRBG") return 0;
    if (u == "BGGR") return 2;
    if (u == "GBRG") return 3;
    return static_cast<uint32_t>(-1);
}


#endif //UTILS_H
