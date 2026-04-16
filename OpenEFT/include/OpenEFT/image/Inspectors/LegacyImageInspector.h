#pragma once
#include <OpenEFT/image/ImageInspector.h>
#include <OpenEFT/image/Types.h>
#include <OpenEFT/image/decoders/TIFFDecoder.h>
#include <OpenEFT/image/decoders/WSQDecoder.h>
#include <OpenEFT/nist/Helpers.h>
#include <OpenEFT/nist/NBISWrapper.h>

namespace OpenEFT {

    class LegacyImageInspector : public ImageInspector {
      public:
        bool CanHandle(const nnist::Record& record, const nnist::Field& field,
                       std::shared_ptr<NMTK::AppContext> ctx) const override {
            return record.is_binary_record && record.type >= 3 &&
                   record.type <= 7;
        }

        ImageInfo Inspect(
            const nnist::Record& record, const nnist::Field& field,
            std::shared_ptr<NMTK::AppContext> ctx) const override {
            ImageInfo info;

            if (!record.is_binary_record || record.raw_record.size() < 18)
                return info;

            const auto& data = record.raw_record;
            const uint8_t* payload = data.data() + 18;
            const size_t payload_size = data.size() - 18;

            info.channels = 1;
            info.valid = false;

            // 1) TIFF embedded in legacy record (real-world cursed data)
            if (FindTiffStart(data.data(), data.size()) != nullptr) {
                info.format = FORMAT::TIFF;
                info.channels = 3;  // because libtiff → RGB output
                info.bit_depth = 8;
                info.valid = true;
                return info;
            }

            // 2) WSQ embedded in Type-3/4
            if ((record.type == 3 || record.type == 4) && payload_size >= 2 &&
                payload[0] == 0xFF && payload[1] == 0xA0) {
                info.format = FORMAT::WSQ;
                info.bit_depth = 8;
                info.valid = true;
                return info;
            }

            // 3) Legacy raw / bit-packed path
            int width = 0;
            int height = 0;
            if (!ExtractLegacyDimensions(record, width, height)) {
                info.error = "Unable to determine legacy image dimensions.";
                return info;
            }

            info.width = width;
            info.height = height;
            info.format = FORMAT::RAW;
            info.bit_depth = (record.type == 5 || record.type == 6) ? 1 : 8;
            info.valid = (width > 0 && height > 0);

            return info;
        }

        bool Decode(const nnist::Record& record, const nnist::Field& field,
                    Image& out,
                    std::shared_ptr<NMTK::AppContext> ctx) const override {
            out = {};

            if (!record.is_binary_record || record.raw_record.size() < 18)
                return false;

            const auto& data = record.raw_record;
            const uint8_t* raw = data.data();
            const size_t total_size = data.size();
            const uint8_t* payload = data.data() + 18;
            const size_t payload_size = data.size() - 18;

            // ---- 1) Try TIFF FIRST (real-world dominant for Type-5/6)
            if (ExtractTiffLibtiff(record, out)) {
                return true;
            }

            // ---- 2) WSQ (Type-3/4)
            if ((record.type == 3 || record.type == 4) && payload_size >= 2 &&
                payload[0] == 0xFF && payload[1] == 0xA0) {
                return DecodeWSQ(payload, payload_size, out);
            }

            // ---- 3) Legacy fallback
            int width = 0;
            int height = 0;
            if (!ExtractLegacyDimensions(record, width, height)) {
                out.info.error = "Unable to determine legacy image dimensions.";
                return false;
            }

            out.info.width = width;
            out.info.height = height;
            out.info.channels = 1;
            out.info.format = FORMAT::RAW;
            out.info.valid = true;

            if (record.type == 5 || record.type == 6) {
                return DecodeBitPacked(payload, payload_size, width, height,
                                       out);
            }

            return DecodeRaw8(payload, payload_size, width, height, out);
        }

      private:
        static bool DecodeRaw8(const uint8_t* payload, size_t payload_size,
                               int width, int height, Image& out) {
            const size_t expected = static_cast<size_t>(width) * height;
            if (payload_size < expected) {
                out.info.error = "Raw payload smaller than expected.";
                out.info.valid = false;
                return false;
            }

            out.info.bit_depth = 8;
            out.pixels.assign(payload, payload + expected);
            return true;
        }

        static bool DecodeBitPacked(const uint8_t* payload, size_t payload_size,
                                    int width, int height, Image& out) {
            const size_t stride = (static_cast<size_t>(width) + 7) / 8;
            const size_t min_required = stride * static_cast<size_t>(height);

            // Real files are messy; allow best-effort if there is enough for at
            // least one row.
            if (payload_size < stride) {
                out.info.error = "Bit-packed payload too small.";
                out.info.valid = false;
                return false;
            }

            const size_t safe_rows =
                std::min(static_cast<size_t>(height), payload_size / stride);

            if (safe_rows == 0) {
                out.info.error = "No complete bit-packed rows available.";
                out.info.valid = false;
                return false;
            }

            if (safe_rows != static_cast<size_t>(height)) {
                out.info.height = static_cast<int>(safe_rows);
            }

            out.info.bit_depth = 1;
            out.pixels.resize(static_cast<size_t>(width) * out.info.height);

            for (int y = 0; y < out.info.height; ++y) {
                const uint8_t* row_src =
                    payload + (static_cast<size_t>(y) * stride);
                uint8_t* row_dst =
                    out.pixels.data() + (static_cast<size_t>(y) * width);

                for (int x = 0; x < width; ++x) {
                    const uint8_t byte = row_src[x / 8];
                    const uint8_t bit = (byte >> (7 - (x % 8))) & 0x01;

                    // 1 = ink/black, 0 = paper/white
                    row_dst[x] = bit ? 0 : 255;
                }
            }

            return true;
        }

        static bool ExtractLegacyDimensions(const nnist::Record& record,
                                            int& width, int& height) {
            width = 0;
            height = 0;

            if (record.raw_record.size() < 18)
                return false;

            const auto& data = record.raw_record;
            const uint16_t w_be = (data[13] << 8) | data[14];
            const uint16_t h_be = (data[15] << 8) | data[16];
            const uint16_t w_le = (data[14] << 8) | data[13];
            const uint16_t h_le = (data[16] << 8) | data[15];

            const size_t payload_size = data.size() - 18;

            // For type 5/6, compare against bit-packed expectation.
            // For type 3/4, compare against raw 8-bit expectation.
            auto score_candidate = [&](uint16_t w, uint16_t h) -> size_t {
                if (w == 0 || h == 0)
                    return SIZE_MAX;

                size_t expected = 0;
                if (record.type == 5 || record.type == 6) {
                    expected = ((static_cast<size_t>(w) + 7) / 8) * h;
                } else {
                    expected = static_cast<size_t>(w) * h;
                }

                return (expected > payload_size) ? (expected - payload_size)
                                                 : (payload_size - expected);
            };

            const size_t err_be = score_candidate(w_be, h_be);
            const size_t err_le = score_candidate(w_le, h_le);

            if (err_be == SIZE_MAX && err_le == SIZE_MAX)
                return false;

            if (err_be <= err_le) {
                width = w_be;
                height = h_be;
            } else {
                width = w_le;
                height = h_le;
            }

            return width > 0 && height > 0;
        }
    };

}  // namespace OpenEFT