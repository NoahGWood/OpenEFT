#pragma once
#include <OpenEFT/image/ImageInspector.h>
#include <OpenEFT/image/Types.h>
#include <OpenEFT/nist/Helpers.h>

extern "C" {
#include <util.h>
#include <wsq.h>
}
#include <stb_image.h>

namespace OpenEFT {
    class SignatureImageInspector : public ImageInspector {
      public:
        bool CanHandle(const nnist::Record& record, const nnist::Field& field,
                       std::shared_ptr<NMTK::AppContext> ctx) const override {
            return record.type == 8 && record.is_binary_record;
        }

        ImageInfo Inspect(
            const nnist::Record& record, const nnist::Field& field,
            std::shared_ptr<NMTK::AppContext> ctx) const override {
            ImageInfo info;
            if (!record.is_binary_record || record.type != 8 ||
                record.raw_record.size() < 12)
                return info;

            const auto& data = record.raw_record;

            // Byte 6 is the Representation Type (SRT)
            uint8_t srt = data[6];
            if (srt == 1) {
                info.valid = false;
                info.error = "Vector signature detected (X,Y coordinates).";
                return info;
            }

            // Standard Type-8 Binary Offsets (Check your specific file's spec!)
            // Note: If your records are 'Tagged' style but in a binary
            // container, these offsets might shift.
            info.width = (data[8] << 8) | data[9];
            info.height = (data[10] << 8) | data[11];

            // Most signatures are 1-bit or 8-bit grayscale
            info.bit_depth = 8;
            info.channels = 1;
            info.format = FORMAT::RAW;

            const uint8_t* payload =
                data.data() + 12;  // Payload starts after H
            size_t payload_size = data.size() - 12;

            // Detect WSQ
            if (payload_size >= 2 && payload[0] == 0xFF && payload[1] == 0xA0) {
                info.format = FORMAT::WSQ;
            }

            info.valid = (info.width > 0 && info.height > 0);
            return info;
        }

        bool Decode(const nnist::Record& record, const nnist::Field& field,
                    Image& out,
                    std::shared_ptr<NMTK::AppContext> ctx) const override {
            // Type-8 is a Binary Record, so we use record.raw_record directly
            const auto& data = record.raw_record;

            if (data.size() < 12)  // Minimum header size for Type-8
                return false;

            // 1. Check Representation Type (Byte 6)
            // 0 = Manual/Raster, 1 = Vector, 2 = Compressed Vector
            uint8_t srt = data[6];
            if (srt != 0 && srt != 1) {  // If it's not raster, we usually can't
                                         // "pixel" decode it yet
                // Note: If you want to support WSQ signatures, srt might be 0
                // but compression 1
            }

            // 2. Extract Dimensions from correct Type-8 offsets
            // Bytes 8-9: Width, Bytes 10-11: Height
            uint16_t width = (data[8] << 8) | data[9];
            uint16_t height = (data[10] << 8) | data[11];

            if (width == 0 || height == 0)
                return false;

            // 3. Define Payload Start
            // In Type-8 Binary, the image data starts at byte 12
            const uint8_t* payload = data.data() + 12;
            size_t payload_size = data.size() - 12;

            ImageInfo info;
            info.width = width;
            info.height = height;
            info.bit_depth = 8;
            info.channels = 1;
            info.valid = true;

            // 4. Handle WSQ Signature (FF A0)
            if (payload_size >= 2 && payload[0] == 0xFF && payload[1] == 0xA0) {
                int w, h, d, ppi, lossy;
                unsigned char* pixels = nullptr;

                // Ensure your wsq_decode_mem is linked correctly
                int ret = wsq_decode_mem(&pixels, &w, &h, &d, &ppi, &lossy,
                                         const_cast<unsigned char*>(payload),
                                         (int)payload_size);

                if (ret == 0 && pixels) {
                    out.info = info;
                    out.info.width = w;
                    out.info.height = h;
                    out.pixels.assign(pixels, pixels + (w * h));
                    free(pixels);
                    return true;
                }
                return false;
            }

            // 5. Handle RAW Raster
            // Signatures are often 1-bit (packed) or 8-bit.
            // If the size is exactly W*H, it's 8-bit grayscale.
            size_t expected_8bit = (size_t)width * height;
            if (payload_size >= expected_8bit) {
                out.info = info;
                out.pixels.assign(payload, payload + expected_8bit);
                return true;
            }

            // 6. Handle 1-bit Packed (Very common in Type-8)
            // If size is (W*H)/8, we need to unpack bits to bytes
            size_t expected_1bit = (expected_8bit + 7) / 8;
            if (payload_size >= expected_1bit) {
                out.info = info;
                out.pixels.resize(expected_8bit);
                for (size_t i = 0; i < expected_8bit; ++i) {
                    uint8_t byte = payload[i / 8];
                    uint8_t bit = (byte >> (7 - (i % 8))) & 0x01;
                    out.pixels[i] =
                        bit ? 0x00 : 0xFF;  // Bit 1 is usually black (ink)
                }
                return true;
            }

            return false;
        }
    };
}  // namespace OpenEFT
