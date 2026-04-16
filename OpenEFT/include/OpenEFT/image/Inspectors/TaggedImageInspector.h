// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <OpenEFT/image/ImageInspector.h>
#include <OpenEFT/image/Types.h>
#include <OpenEFT/image/decoders/JPEGDecoder.h>
#include <OpenEFT/image/decoders/JP2KDecoder.h>
#include <OpenEFT/image/decoders/WSQDecoder.h>
#include <OpenEFT/nist/Helpers.h>

namespace OpenEFT {

    class TaggedImageInspector : public ImageInspector {
      public:
        bool CanHandle(const nnist::Record& record, const nnist::Field& field,
                       std::shared_ptr<NMTK::AppContext> ctx) const override {
            // Handles most type 17, 16, 15, 14, 13 records
            uint32_t mask = 0x3E400;
            if (!((1 << record.type) & mask))
                return false;

            std::string tag = DecodeText(field.tag);
            return tag.ends_with(".999");
        }

        ImageInfo Inspect(
            const nnist::Record& record, const nnist::Field& field,
            std::shared_ptr<NMTK::AppContext> ctx) const override {
            ImageInfo info;
            info.format = FORMAT::UNKNOWN;

            for (const auto& f : record.fields) {
                if (f.subfields.empty() || f.subfields[0].items.empty())
                    continue;

                int suffix = GetTagSuffix(f.tag);
                const auto& bytes = f.subfields[0].items[0].bytes;

                try {
                    switch (suffix) {
                        case 6:
                            info.width = std::stoi(DecodeEnum(bytes));
                            break;
                        case 7:
                            info.height = std::stoi(DecodeEnum(bytes));
                            break;
                        case 11: {
                            std::string compression = DecodeEnum(bytes);
                            if (compression == "WSQ" ||
                                compression == "WSQ20") {
                                info.format = FORMAT::WSQ;
                            } else if (compression == "JPEG" ||
                                       compression == "JPEGB") {
                                info.format = FORMAT::JPEG;
                            } else if (compression == "JP2K") {
                                info.format = FORMAT::JP2K;
                            } else if (compression == "PNG") {
                                info.format = FORMAT::PNG;
                            } else if (compression == "NONE") {
                                info.format = FORMAT::RAW;
                            }
                            break;
                        }
                        case 12:
                            if (record.type == 10) {
                                // Tag 10.012 is Color Space (csp)
                                std::string csp = DecodeEnum(bytes);
                                info.channels = (csp == "SRGB" ||
                                                 csp == "YCC" || csp == "RGB")
                                                    ? 3
                                                    : 1;
                                info.bit_depth = 8;  // Default for Type-10 if
                                                     // .026 is missing
                            } else {
                                // Most others use x.0012 as Bit Depth (BDP)
                                // (because the best standards, aren't)
                                info.bit_depth = std::stoi(DecodeEnum(bytes));
                            }
                            break;
                        case 13: {
                            if (record.type == 10) {
                                // 10.013 is Pixel Scale, ignore
                            } else {
                                std::string val = DecodeEnum(bytes);
                                info.channels =
                                    (val == "RGB" || val == "SRGB") ? 3 : 1;
                            }
                            break;
                        }
                    }
                } catch (const std::invalid_argument& e) {
                    CORE_ERROR("Tag {}.{:03} has invalid numeric data: '{}'",
                               record.type, suffix, DecodeEnum(bytes));
                } catch (const std::out_of_range& e) {
                    CORE_ERROR("Tag {}.{:03} value is out of range.",
                               record.type, suffix);
                } catch (...) {
                    CORE_ERROR("Unexpected error parsing tag {}.{:03}",
                               record.type, suffix);
                }
            }
            info.valid = (info.width > 0 && info.height > 0);
            return info;
        }

        bool Decode(const nnist::Record& record, const nnist::Field& field,
                    Image& out,
                    std::shared_ptr<NMTK::AppContext> ctx) const override {
            auto binary = FindBinaryStart(field.raw_field);
            if (!binary.data || binary.size == 0)
                return false;

            ImageInfo info = Inspect(record, field, ctx);

            const uint8_t* data = (const uint8_t*)field.raw_field.data();
            size_t size = field.raw_field.size();

            switch (info.format) {
                case FORMAT::UNKNOWN: {
                    info.error = "Unknown file format.";
                    return false;
                }
                case FORMAT::WSQ: {
                    return DecodeWSQ(data, size, out);
                }
                case FORMAT::JPEG: {
                    return DecodeJPEG(data, size, out);
                }
                case FORMAT::JP2K: {
                    return DecodeJP2K(data, size, out);
                }
                case FORMAT::RAW: {
                    size_t expected =
                        (size_t)info.width * (size_t)info.height *
                        (size_t)(info.channels ? info.channels : 1);

                    out.info = info;
                    if (binary.size < expected) {
                        info.valid = false;
                        info.error = "Binary size is less than expected size.";
                        return false;
                    }
                    out.info = info;
                    info.valid = true;
                    out.pixels.assign(binary.data, binary.data + expected);
                    return true;
                }
                default:
                    break;
            }
            return false;
        }
    };

}  // namespace OpenEFT