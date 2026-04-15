#pragma once
#include <OpenEFT/image/ImageInspector.h>
#include <OpenEFT/image/Types.h>
#include <OpenEFT/nist/Helpers.h>

namespace OpenEFT {

    class RawImageInspector : public ImageInspector {
      public:
        bool CanHandle(const nnist::Record& record, const nnist::Field& field,
                       std::shared_ptr<NMTK::AppContext> ctx) const override {
            if (record.is_binary_record) {
                return true;
            }

            std::string tag = DecodeText(field.tag);

            // Only binary payload fields
            if (!tag.ends_with(".999")) {
                return false;
            }

            // Check compression field (.011)
            for (const auto& f : record.fields) {
                if (f.subfields.empty() || f.subfields[0].items.empty())
                    continue;

                int suffix = GetTagSuffix(f.tag);
                if (suffix == 11) {
                    std::string comp =
                        DecodeEnum(f.subfields[0].items[0].bytes);
                    if (comp == "NONE" || comp == "RAW")
                        return true;
                }
            }

            // Fallback: Type-17 (raw iris etc.)
            return record.type == 17;
        }

        ImageInfo Inspect(
            const nnist::Record& record, const nnist::Field& field,
            std::shared_ptr<NMTK::AppContext> ctx) const override {
            ImageInfo info;
            info.format = FORMAT::RAW;

            if (record.is_binary_record) {
                if (record.raw_record.size() < 18) {
                    return info;
                }
                info.width =
                    (record.raw_record[13] << 8) | record.raw_record[14];
                info.height =
                    (record.raw_record[15] << 8) | record.raw_record[16];
                info.bit_depth = 8;
                info.channels = 1;
                info.valid = (info.width > 0 && info.height > 0);
                return info;
            }

            for (const auto& f : record.fields) {
                if (f.subfields.empty() || f.subfields[0].items.empty())
                    continue;

                int suffix = GetTagSuffix(f.tag);
                const auto& bytes = f.subfields[0].items[0].bytes;

                try {
                    switch (suffix) {
                        case 6:  // width
                            info.width = std::stoi(DecodeEnum(bytes));
                            break;
                        case 7:  // height
                            info.height = std::stoi(DecodeEnum(bytes));
                            break;
                        case 12:  // bit depth
                            info.bit_depth = std::stoi(DecodeEnum(bytes));
                            break;
                        case 13: {  // color space
                            std::string val = DecodeEnum(bytes);
                            if (val == "SRGB" || val == "RGB")
                                info.channels = 3;
                            // info.channels = 1;
                            else
                                info.channels = 1;
                            break;
                        }
                        default:
                            break;
                    }
                } catch (...) {
                    // ignore bad conversions
                }
            }

            // Basic validation
            info.valid = (info.width > 0 && info.height > 0 &&
                          info.channels > 0 && info.bit_depth > 0);

            return info;
        }

        bool Decode(const nnist::Record& record, const nnist::Field& field,
                    Image& out,
                    std::shared_ptr<NMTK::AppContext> ctx) const override {
            ImageInfo info = Inspect(record, field, ctx);
            if (!info.valid)
                return false;

            // if(record.is_binary_record) {
            //     out.pixels 
            // }

            auto binary = FindBinaryStart(field.raw_field);
            if (!binary.data || binary.size == 0)
                return false;

            size_t expected = static_cast<size_t>(info.width) *
                              static_cast<size_t>(info.height) *
                              static_cast<size_t>(info.channels);

            // if (binary.size < expected)
            //     return false;
            if (binary.size == expected) {
                out.pixels.assign(binary.data, binary.data + expected);
                out.info = info;
                return true;
            }
            size_t stride = binary.size / info.height;
            // sanity check (important)
            if (stride < info.width * info.channels) {
                return false;
            }
            out.info = info;
            std::vector<uint8_t> tight;
            tight.reserve(expected);
            for (int y = 0; y < info.height; ++y) {
                const uint8_t* row = binary.data + y * stride;

                tight.insert(tight.end(), row,
                             row + (info.width * info.channels));
            }
            out.pixels = std::move(tight);
            out.info = info;
            return true;
        }
    };

}  // namespace OpenEFT

// namespace OpenEFT {
//     class RawImageInspector : public ImageInspector {
//       public:
//         // Handles cases where compression is "NONE" or "RAW", or Type-17
//         bool CanHandle(const nnist::Record& record, const nnist::Field&
//         field,
//                        std::shared_ptr<NMTK::AppContext> ctx) const override
//                        {
//             // Check if it's a binary field (usually .999)
//             std::string tag = DecodeText(field.tag);
//             if (!tag.ends_with(".999"))
//                 return false;

//             // Look for a "NONE" or "RAW" string in the compression field
//             // (usually .011)
//             for (const auto& f : record.fields) {
//                 std::string t = DecodeText(f.tag);
//                 if (t.ends_with(".011") && !f.subfields.empty()) {
//                     std::string comp =
//                         DecodeText(f.subfields[0].items[0].bytes);
//                     if (comp == "NONE" || comp == "RAW")
//                         return true;
//                 }
//             }

//             // Fallback: Type-17 is almost always raw iris data
//             return record.type == 17;
//         }

//         ImageInfo Inspect(
//             const nnist::Record& record, const nnist::Field& field,
//             std::shared_ptr<NMTK::AppContext> ctx) const override {
//             ImageInfo info;

//             // Pull Metadata from sibling fields
//             for (const auto& f : record.fields) {
//                 std::string tag = DecodeText(f.tag);
//                 if (f.subfields.empty() || f.subfields[0].items.empty())
//                     continue;
//                 const auto& val = DecodeText(f.subfields[0].items[0].bytes);

//                 if (tag.ends_with(".006"))
//                     info.width = std::stoi(val);
//                 if (tag.ends_with(".007"))
//                     info.height = std::stoi(val);
//                 if (tag.ends_with(".012"))
//                     info.bit_depth = std::stoi(val);
//                 if (tag.ends_with(".013")) {
//                     if (val == "SRGB" || val == "RGB")
//                         info.channels = 3;
//                     else
//                         info.channels = 1;
//                 }
//             }

//             info.format = FORMAT::RAW;
//             info.valid = (info.width > 0 && info.height > 0);
//             return info;
//         }

//         bool Decode(const nnist::Record& record, const nnist::Field& field,
//                     Image& out,
//                     std::shared_ptr<NMTK::AppContext> ctx) const override {
//             ImageInfo info = Inspect(record, field, nullptr);
//             if (!info.valid)
//                 return false;

//             // Use your FindBinaryStart logic to skip the "17.999:" prefix
//             auto binary = FindBinaryStart(field.raw_field);
//             if (!binary.data)
//                 return false;

//             out.info = info;
//             out.pixels.assign(binary.data, binary.data + binary.size);

//             return true;
//         }
//     };
// }  // namespace OpenEFT
