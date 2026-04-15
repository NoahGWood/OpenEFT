#pragma once
#include <nnist/nnist.h>

#include <cctype>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace OpenEFT {
    inline std::string NormalizeFieldId(const std::string& raw) {
        // Expect forms like "1.03", "1.003", "10.1", etc.
        auto dot = raw.find('.');
        if (dot == std::string::npos)
            return raw;

        std::string rec = raw.substr(0, dot);
        std::string fld = raw.substr(dot + 1);

        // trim whitespace
        while (!fld.empty() && std::isspace((unsigned char)fld.back()))
            fld.pop_back();
        while (!fld.empty() && std::isspace((unsigned char)fld.front()))
            fld.erase(fld.begin());

        // left-pad field number to 3 digits
        while (fld.size() < 3)
            fld.insert(fld.begin(), '0');

        return rec + "." + fld;
    }

    inline std::string DecodeText(const std::vector<uint8_t>& bytes) {
        std::string out;
        out.reserve(bytes.size());

        for (uint8_t b : bytes) {
            if (b >= 0x20 && b <= 0x7E) {
                out.push_back(static_cast<char>(b));
            } else {
                out.push_back('.');
            }
        }

        return out;
    }

    inline std::string DecodeEnum(const std::vector<uint8_t>& bytes) {
        return std::string(bytes.begin(), bytes.end());
    }

    inline std::string HexDump(const std::vector<uint8_t>& bytes) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');

        for (size_t i = 0; i < bytes.size(); ++i) {
            oss << std::setw(2) << static_cast<int>(bytes[i]);
            if (i + 1 < bytes.size())
                oss << ' ';
        }

        return oss.str();
    }
    struct BinaryResult {
        const uint8_t* data = nullptr;
        size_t size = 0;
        std::string format = "RAW";  // Default to raw for Type-17
    };

    // Do NOT use the dataptr for compressed formats
    inline BinaryResult FindBinaryStart(const std::vector<uint8_t>& raw_field) {
        BinaryResult result;
        if (raw_field.empty())
            return result;

        // Find the LAST colon in the field
        size_t start_pos = 0;
        for (size_t i = 0; i < raw_field.size(); ++i) {
            if (raw_field[i] == ':')
                start_pos = i + 1;
        }

        // Skip any immediate whitespace or NIST separators (GS/RS/US)
        while (start_pos < raw_field.size() && raw_field[start_pos] < 0x20) {
            start_pos++;
        }

        if (start_pos >= raw_field.size())
            return result;

        result.data = &raw_field[start_pos];
        result.size = raw_field.size() - start_pos;

        // Quick format check
        if (result.size > 2) {
            if (result.data[0] == 0xFF && result.data[1] == 0xA0)
                result.format = "WSQ";
            else if (result.data[0] == 0xFF && result.data[1] == 0xD8)
                result.format = "JPEG";
            else
                result.format = "RAW";
        }

        return result;
    }

    // inline BinaryResult FindBinaryStart(const std::vector<uint8_t>&
    // raw_field) {
    //     BinaryResult result;
    //     if (raw_field.empty())
    //         return result;

    //     // 1. Find the actual start of the binary payload.
    //     // In NIST/EFT, the binary data usually follows the LAST colon of the
    //     // tag prefix.
    //     size_t last_colon = std::string::npos;
    //     for (size_t i = 0; i < raw_field.size(); ++i) {
    //         if (raw_field[i] == ':') {
    //             last_colon = i;
    //             // We don't break; we want the LAST colon in case of
    //             // 14.999:item1<GS>14.999:binary
    //         }
    //         // If we hit a known binary header early, stop and use the
    //         previous
    //         // colon
    //         if (i + 1 < raw_field.size()) {
    //             if (raw_field[i] == 0xFF &&
    //                 (raw_field[i + 1] == 0xA0 || raw_field[i + 1] == 0xD8)) {
    //                 break;
    //             }
    //         }
    //     }

    //     if (last_colon == std::string::npos ||
    //         last_colon + 1 >= raw_field.size()) {
    //         return result;
    //     }

    //     const uint8_t* start_ptr = raw_field.data() + last_colon + 1;
    //     size_t remaining = raw_field.size() - (last_colon + 1);

    //     // 2. Check for compressed signatures IMMEDIATELY.
    //     // We only scan a small window (e.g., 32 bytes) because headers
    //     // should be right at the start of the payload.
    //     size_t scan_limit = std::min<size_t>(remaining, 32);
    //     for (size_t offset = 0; offset < scan_limit; ++offset) {
    //         const uint8_t* p = start_ptr + offset;

    //         // WSQ (FF A0)
    //         if (p[0] == 0xFF && p[1] == 0xA0) {
    //             result.data = p;
    //             result.size = remaining - offset;
    //             result.format = "WSQ";
    //             return result;
    //         }
    //         // JPEG (FF D8)
    //         if (p[0] == 0xFF && p[1] == 0xD8) {
    //             result.data = p;
    //             result.size = remaining - offset;
    //             result.format = "JPEG";
    //             return result;
    //         }
    //         // JP2K (00 00 00 0C)
    //         if (offset + 4 <= remaining && p[0] == 0x00 && p[1] == 0x00 &&
    //             p[2] == 0x00 && p[3] == 0x0C) {
    //             result.data = p;
    //             result.size = remaining - offset;
    //             result.format = "JP2K";
    //             return result;
    //         }
    //     }

    //     // 3. Fallback for RAW (Type-17 sRGB)
    //     // If we didn't find a magic header in the first 32 bytes,
    //     // it's raw pixels starting right after the colon.
    //     result.data = start_ptr;
    //     result.size = remaining;
    //     result.format = "RAW";
    //     return result;
    // }

    // inline BinaryResult FindBinaryStart(const std::vector<uint8_t>&
    // raw_field) {
    //     BinaryResult result;
    //     if (raw_field.empty())
    //         return result;

    //     size_t colon = std::string::npos;
    //     size_t limit = raw_field.size();

    //     for (size_t i = 0; i < limit; ++i) {
    //         uint8_t b = raw_field[i];
    //         if (b == ':') {
    //             colon = i;
    //             break;
    //         }
    //         // stop if we hit a non-ASCII control char (start of binary)
    //         if (b < 0x20 && b != '\t' && b != '\n' && b != '\r') {
    //             break;
    //         }
    //     }

    //     if (colon == std::string::npos || colon + 1 >= raw_field.size()) {
    //         return result;
    //     }

    //     // 2. Start scanning from after that colon
    //     const uint8_t* search_ptr = raw_field.data() + colon + 1;
    //     size_t remaining = raw_field.size() - (colon + 1);

    //     // 3. Signature Detection Loop
    //     // We look ahead a few bytes to see if a compressed header starts
    //     for (size_t offset = 0; offset < remaining - 4; ++offset) {
    //         const uint8_t* p = search_ptr + offset;
    //         // NEW: detect ASCII prefix
    //         if (offset + 5 <= remaining) {
    //             if (memcmp(p, "WSQ20", 5) == 0) {
    //                 result.data = p + 5;
    //                 result.size = remaining - (offset + 5);
    //                 result.format = "WSQ";
    //                 return result;
    //             }
    //         }
    //         // WSQ: FF A0
    //         // WSQ markers
    //         if ((p[0] == 0xFF && p[1] == 0xA0) ||  // SOI
    //             (p[0] == 0xFF && p[1] == 0xA1)) {  // alternate marker
    //             result.data = p;
    //             result.size = remaining - offset;
    //             result.format = "WSQ";
    //             return result;
    //         }
    //         // JPEG: FF D8
    //         if (p[0] == 0xFF && p[1] == 0xD8) {
    //             result.data = p;
    //             result.size = remaining - offset;
    //             result.format = "JPEG";
    //             return result;
    //         }
    //         // JP2K Signature Box: 00 00 00 0C
    //         if (p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x00 && p[3] == 0x0C)
    //         {
    //             result.data = p;
    //             result.size = remaining - offset;
    //             result.format = "JP2K";
    //             return result;
    //         }
    //     }

    //     // 4. Fallback: If no signatures found, it's likely Raw sRGB
    //     (Type-17)
    //     // We return the data starting immediately after the colon
    //     result.data = search_ptr;
    //     result.size = remaining;
    //     result.format = "RAW";
    //     return result;
    // }
    // Helper to find the numeric value of a tag suffix (e.g., "006" -> 6)
    inline int GetTagSuffix(const std::vector<uint8_t>& tag_bytes) {
        std::string tag = DecodeText(tag_bytes);
        size_t dot = tag.find_last_of('.');
        if (dot == std::string::npos)
            return -1;
        try {
            return std::stoi(tag.substr(dot + 1));
        } catch (...) {
            return -1;
        }
    }
}  // namespace OpenEFT
