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
    // If utf8Len is 0, calculate string length automatically (excluding '\0')
    int actualUtf8Len = (utf8Len == 0) ? lstrlenA(utf8Str) : utf8Len;
    // Use Windows MultiByteToWideChar with UTF-8 code page
    return MultiByteToWideChar(
        CP_UTF8,          // source encoding: UTF-8
        0,                // flags (0 = default)
        utf8Str,          // source UTF-8 string
        actualUtf8Len,    // source string length (-1 = null-terminated)
        wStr,             // destination wide-char buffer
        wStrLen           // max length of destination buffer (in wchar_t)
    );
}
# endif

// Convert dng_string to std::string
inline std::string DNGStringToStdString(const dng_string& dngStr) {
    if (dngStr.IsEmpty()) {
        return "";
    }
    return std::string(dngStr.Get());
}

// Convert dng_urational to double
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
