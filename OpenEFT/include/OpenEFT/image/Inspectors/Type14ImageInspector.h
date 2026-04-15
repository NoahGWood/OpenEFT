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

    class Type14ImageInspector : public ImageInspector {
      public:
        bool CanHandle(const nnist::Record& record, const nnist::Field& field,
                       std::shared_ptr<NMTK::AppContext> ctx) const override {
            if (record.type != 14)
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
                            }
                            break;
                        }
                        case 12:
                            info.bit_depth = std::stoi(DecodeEnum(bytes));
                            break;
                        case 13: {
                            std::string val = DecodeEnum(bytes);
                            info.channels =
                                (val == "RGB" || val == "SRGB") ? 3 : 1;
                            break;
                        }
                    }
                } catch (...) {
                    CORE_ERROR(
                        "Major shit be happening now, we don't know wtf, but "
                        "it's broken");
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

            switch (info.format) {
                case FORMAT::UNKNOWN: {
                    info.error = "Unknown file format.";
                    return false;
                }
                case FORMAT::WSQ: {
                    int w, h, d, ppi, lossy;
                    unsigned char* pixels = nullptr;
                    // Make sure we don't use the FindBinaryStart fx for wsq
                    const uint8_t* data =
                        (const uint8_t*)field.raw_field.data();
                    size_t size = field.raw_field.size();

                    // Hard Sync: Find the actual start within the provided blob
                    const uint8_t* wsq_start = nullptr;
                    for (size_t i = 0; i < size - 1; ++i) {
                        if (data[i] == 0xFF &&
                            data[i + 1] == 0xA0) {
                            wsq_start = data + i;
                            break;
                        }
                    }
                    if (!wsq_start) {
                        info.valid = false;
                        info.error =
                            "Could not find WSQ SOI (FF A0) in binary data.";
                        out.info = info;
                        return false;
                    }
                    // Calculate remaining size from the found start
                    size_t actual_size =
                        size - (wsq_start - data);
                    int ret =
                        wsq_decode_mem(&pixels, &w, &h, &d, &ppi, &lossy,
                                       const_cast<unsigned char*>(wsq_start),
                                       (int)actual_size);
                    info.width = w;
                    info.height = h;
                    info.bit_depth = d;
                    info.channels = 1;
                    info.format = FORMAT::WSQ;
                    if (ret != 0 || !pixels) {
                        out.info = info;
                        out.info.valid = false;
                        out.info.error = "WSQ Decoder rejected stream (Error " +
                                         std::to_string(ret) + ")";
                        return false;
                    }
                    info.valid = true;
                    out.info = info;
                    out.pixels.assign(pixels, pixels + w*h);
                    free(pixels);
                    return true;
                }
                case FORMAT::JPEG: {
                    int w, h, ch;

                    const uint8_t* data =
                        (const uint8_t*)field.raw_field.data();
                    size_t size = field.raw_field.size();
                    unsigned char* pixels = stbi_load_from_memory(
                        data, (int)size, &w, &h, &ch, 0);

                    if (!pixels)
                        return false;

                    info.width = w;
                    info.height = h;
                    info.channels = (ch == 3) ? 3 : 1;
                    info.bit_depth = 8;
                    info.format = FORMAT::JPEG;
                    info.valid = true;

                    size_t total = (size_t)w * h * info.channels;

                    out.info = info;
                    out.pixels.assign(pixels, pixels + total);

                    stbi_image_free(pixels);
                    return true;
                }
                case FORMAT::JP2K: {
                    info.error = "Not implemented.";
                    out.info = info;
                    return false;
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