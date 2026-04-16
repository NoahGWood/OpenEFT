// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace OpenEFT {
    enum class FORMAT : uint8_t {
        UNKNOWN,
        WSQ,
        JP2K,
        JPEG,
        PNG,
        TIFF,
        RAW
    };

    struct ImageInfo {
        FORMAT format = FORMAT::UNKNOWN;  // Source format

        int width = 0;
        int height = 0;

        int bit_depth = 0;  // Decoded depth
        int channels = 0;   // Decoded channels

        int stride = 0;
        int ppi = 0;

        bool valid = false;
        std::string error;
    };

    struct Image {
        ImageInfo info;
        std::vector<uint8_t> pixels;  // Always RGBA8

        inline size_t SizeBytes() const { return pixels.size(); }
        bool IsValid() const { return info.valid && !pixels.empty(); }
    };

    struct FieldUpdate {
        std::string tag;
        std::string value;
        bool required = false;
        bool conflict = false;
        std::string reason;
    };

    struct SuggestionReport {
        std::vector<FieldUpdate> updates;
        std::vector<std::string> warnings;
    };

}  // namespace OpenEFT
