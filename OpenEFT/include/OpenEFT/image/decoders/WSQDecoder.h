#pragma once
#include <OpenEFT/image/Types.h>
#include <OpenEFT/nist/Helpers.h>

extern "C" {
#include <util.h>
#include <wsq.h>
}

namespace OpenEFT {
    inline bool DecodeWSQ(const uint8_t* data, size_t size, Image& out) {
        int w, h, d, ppi, lossy;
        unsigned char* pixels = nullptr;
        // Hard Sync: Find the REAL start within the provided blob
        const uint8_t* wsq_start = nullptr;
        for (size_t i = 0; i < size - 1; ++i) {
            if (data[i] == 0xFF && data[i + 1] == 0xA0) {
                wsq_start = data + i;
                break;
            }
        }
        if (!wsq_start) {
            out.info.valid = false;
            out.info.error = "Could not find WSQ SOI (FF A0) in binary data.";
            return false;
        }
        // Calculate remaining size from the found start
        size_t actual_size = size - (wsq_start - data);
        int ret = wsq_decode_mem(&pixels, &w, &h, &d, &ppi, &lossy,
                                 const_cast<unsigned char*>(wsq_start),
                                 (int)actual_size);
        out.info.width = w;
        out.info.height = h;
        out.info.bit_depth = d;
        out.info.channels = 1;
        out.info.format = FORMAT::WSQ;
        if (ret != 0 || !pixels) {
            out.info.valid = false;
            out.info.error = "WSQ Decoder rejected stream (Error " +
                             std::to_string(ret) + ")";
            return false;
        }
        out.info.valid = true;
        out.pixels.assign(pixels, pixels + w * h);
        free(pixels);
        return true;
    }
}  // namespace OpenEFT
