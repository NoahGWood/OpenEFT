// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <OpenEFT/image/Types.h>
#include <OpenEFT/nist/Helpers.h>
#include <stb_image.h>

namespace OpenEFT {
    inline bool DecodeJPEG(const uint8_t* data, size_t size, Image& out) {
        int w, h, ch;
        // Hard sync: Look for JPEG SOI (FF D8)
        const uint8_t* jpeg_start = nullptr;
        for (size_t i = 0; i < size - 1; ++i) {
            if (data[i] == 0xFF && data[i + 1] == 0xD8) {
                jpeg_start = data + i;
                size -= i;
                break;
            }
        }
        if (!jpeg_start) {
            // Fallback: if we don't find FF D8, maybe stbi can find it. Syncing
            // is still safer tho because people like LEAD technologies just be
            // throwing random ass comments into shit
            jpeg_start = data;
        }
        unsigned char* pixels =
            stbi_load_from_memory(jpeg_start, (int)size, &w, &h, &ch, 0);

        if (!pixels) {
            out.info.valid = false;
            out.info.error = "Failed to extract pixels from JPEG image.";
            return false;
        }

        out.info.width = w;
        out.info.height = h;
        out.info.channels = (ch == 3) ? 3 : 1;
        out.info.bit_depth = 8;
        out.info.format = FORMAT::JPEG;
        out.info.valid = true;

        size_t total = (size_t)w * h * out.info.channels;

        out.pixels.assign(pixels, pixels + total);

        stbi_image_free(pixels);
        return true;
    }
}  // namespace OpenEFT
