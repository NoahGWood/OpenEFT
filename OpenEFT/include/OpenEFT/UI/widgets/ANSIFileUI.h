// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <OpenEFT/core/Project.h>
#include <OpenEFT/nist/Helpers.h>
#include <OpenEFT/rules/RulesUI.h>
#include <nnist/nnist.h>
#include <stb_image.h>

namespace OpenEFT {

    inline void RenderRawBinaryImage(nnist::Record& record) {
        static std::unordered_map<int, std::unique_ptr<NMTK::Texture>>
            binaryCache;

        if (record.raw_record.size() < 18) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1),
                               "Error: Binary record too small.");
            return;
        }

        // 1. Extract Dimensions (Big-Endian)
        // Offset 13-14 is Width, 15-16 is Height
        uint16_t width = (record.raw_record[13] << 8) | record.raw_record[14];
        uint16_t height = (record.raw_record[15] << 8) | record.raw_record[16];
        uint8_t compression = record.raw_record[17];

        // 2. Manage Texture Cache
        if (binaryCache.find(record.idc) == binaryCache.end()) {
            const uint8_t* pixelStart = record.raw_record.data() + 18;

            if (compression == 0) {  // Raw Uncompressed Grayscale
                binaryCache[record.idc] =
                    std::make_unique<NMTK::Texture>(width, height, pixelStart);
            } else if (compression == 1) {
                // TODO: Call your WSQ Decoder here before passing to
                // NMTK::Texture
                ImGui::Text("WSQ Compression detected. Decoding required.");
            }
        }

        // 3. Render in Inspector
        if (binaryCache.count(record.idc)) {
            ImGui::Text("Type-4 Grayscale (%d x %d)", width, height);

            float availWidth = ImGui::GetContentRegionAvail().x;
            float displayHeight = availWidth * ((float)height / (float)width);

            ImGui::Image(binaryCache[record.idc]->GetImGuiID(),
                         ImVec2(availWidth, displayHeight));
        }
    }
    

    inline void RenderType17(nnist::Record* record, nnist::Field* field) {
        uint32_t w = 0, h = 0;
        bool isRGB = false;

        // 1. Dynamic Metadata Extraction
        for (auto& f : record->fields) {
            int suffix = GetTagSuffix(f.tag);
            if (f.subfields.empty() || f.subfields[0].items.empty())
                continue;
            const auto& val = DecodeText(f.subfields[0].items[0].bytes);

            if (suffix == 6)
                w = std::stoul(val);
            if (suffix == 7)
                h = std::stoul(val);
            if (suffix == 13 && val == "SRGB")
                isRGB = true;
        }

        // 2. Locate pixels using our magic-byte scanner
        auto result = FindBinaryStart(field->raw_field);
        if (!result.data || w == 0 || h == 0) {
            ImGui::TextDisabled("Type-17: Missing dimensions or data.");
            return;
        }

        // 3. Texture Management
        static std::unordered_map<nnist::Field*, std::unique_ptr<NMTK::Texture>>
            irisCache;
        if (irisCache.find(field) == irisCache.end()) {
            auto tex = std::make_unique<NMTK::Texture>(w, h, nullptr);
            glBindTexture(GL_TEXTURE_2D, tex->GetID());
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            if (isRGB) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
                             GL_UNSIGNED_BYTE, result.data);
                GLint swizzle[] = {GL_RED, GL_GREEN, GL_BLUE, GL_ONE};
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA,
                                 swizzle);
            } else {
                // Fallback to grayscale if not SRGB
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED,
                             GL_UNSIGNED_BYTE, result.data);
            }
            irisCache[field] = std::move(tex);
        }

        ImGui::Text("Iris (Type-17) %dx%d [%s]", w, h, isRGB ? "RGB" : "GRAY");
        ImGui::Image(irisCache[field]->GetImGuiID(), ImVec2(256, 256));
    }
    inline bool RenderRawGrayscale(uint32_t w, uint32_t h,
                                   const BinaryResult& result) {
        size_t expected = (size_t)w * (size_t)h;

        if (result.size < expected) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1),
                               "RAW size mismatch: expected %zu, got %zu",
                               expected, result.size);
            return false;
        }

        static std::unordered_map<const void*, std::unique_ptr<NMTK::Texture>>
            cache;

        if (!cache.count(result.data)) {
            auto tex = std::make_unique<NMTK::Texture>(w, h, nullptr);

            glBindTexture(GL_TEXTURE_2D, tex->GetID());
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED,
                         GL_UNSIGNED_BYTE, result.data);

            GLint swizzle[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);

            cache[result.data] = std::move(tex);
        }

        ImGui::Image(cache[result.data]->GetImGuiID(), ImVec2(256, 256));
        return true;
    }
    inline void RenderBinaryFieldEditor(nnist::Record* record,
                                        nnist::Field* field) {
        if (!field || field->raw_field.empty())
            return;

        // Route Type-17 to its specialized renderer
        if (record->type == 17) {
            RenderType17(record, field);
            return;
        }

        // 1. Extract dimensions and compression from record
        uint32_t w = 0, h = 0;
        std::string compStr = "RAW";
        for (auto& f : record->fields) {
            int suffix = GetTagSuffix(f.tag);
            if (f.subfields.empty() || f.subfields[0].items.empty())
                continue;
            const auto& val = DecodeText(f.subfields[0].items[0].bytes);

            if (suffix == 6)
                w = std::stoul(val);
            if (suffix == 7)
                h = std::stoul(val);
            if (suffix == 11 || suffix == 12)
                compStr = val;
        }

        // 2. Scan for the image payload
        auto result = FindBinaryStart(field->raw_field);
        if (!result.data) {
            ImGui::TextDisabled("No binary payload found in field.");
            return;
        }

        // 3. Texture Cache Logic
        static std::unordered_map<nnist::Field*, std::unique_ptr<NMTK::Texture>>
            binCache;

        if (binCache.find(field) == binCache.end()) {
            if (result.format == "JPEG") {
                int jW, jH, jC;
                unsigned char* decoded = stbi_load_from_memory(
                    result.data, (int)result.size, &jW, &jH, &jC, 1);
                if (decoded) {
                    binCache[field] =
                        std::make_unique<NMTK::Texture>(jW, jH, decoded);
                    stbi_image_free(decoded);
                }
            } else if (result.format == "WSQ") {
                ImGui::TextColored(ImVec4(1, 1, 0, 1),
                                   "WSQ detected - Decoder needed.");
            } else if (w > 0 && h > 0) {
                // Raw Grayscale (Type-4/14 uncompressed)
                binCache[field] =
                    std::make_unique<NMTK::Texture>(w, h, result.data);
            }
        }

        // 4. Render UI
        if (binCache.count(field)) {
            ImGui::Text("%s Image: %dx%d", result.format.c_str(), w, h);
            ImGui::Image(binCache[field]->GetImGuiID(), ImVec2(256, 256));
        } else {
            ImGui::Text("Binary Field: %s (%zu bytes)", result.format.c_str(),
                        result.size);
            if (result.format == "RAW") {
                RenderRawGrayscale(w, h, result);
            }
        }
    }
    // inline void RenderType17(nnist::Record* record, nnist::Field* field) {
    //     // 1. Get metadata (W/H/ColorSpace)
    //     uint32_t w = 201;  // Should be pulled from 17.006
    //     uint32_t h = 201;  // Should be pulled from 17.007

    //     // 2. Locate pixels after "17.999:"
    //     const uint8_t* pixelData = FindBinaryStart(field->raw_field).data;
    //     if (!pixelData)
    //         return;

    //     // 3. Texture logic
    //     static std::unordered_map<nnist::Field*,
    //     std::unique_ptr<NMTK::Texture>>
    //         irisCache;

    //     if (irisCache.find(field) == irisCache.end()) {
    //         auto tex = std::make_unique<NMTK::Texture>(w, h, nullptr);

    //         // We need a specific RGB upload for NMTK::Texture
    //         glBindTexture(GL_TEXTURE_2D, tex->GetID());
    //         glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // Crucial for width 201!

    //         // Upload 3-channel data
    //         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
    //                      GL_UNSIGNED_BYTE, pixelData);

    //         // Ensure swizzle is standard (R,G,B,A) and not the grayscale
    //         // (R,R,R,1)
    //         GLint swizzleMask[] = {GL_RED, GL_GREEN, GL_BLUE, GL_ONE};
    //         glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA,
    //                          swizzleMask);

    //         irisCache[field] = std::move(tex);
    //     }

    //     // 4. Draw
    //     ImGui::Text("Iris Image (Type-17) - Raw sRGB %dx%d", w, h);
    //     ImGui::Image(irisCache[field]->GetImGuiID(), ImVec2(256, 256));
    // }
    // inline void RenderBinaryFieldEditor(nnist::Record* record,
    //                                     nnist::Field* field) {
    //     if (!field || field->raw_field.empty())
    //         return;
    //     if (record->type == 17) {
    //         RenderType17(record, field);
    //         return;
    //     }

    //     // 1. Metadata lookup (The instructions)
    //     uint32_t w = 0, h = 0;
    //     std::string compression = "RAW";

    //     // Inside RenderBinaryFieldEditor
    //     for (auto& f : record->fields) {
    //         std::string tag = DecodeText(f.tag);
    //         if (f.subfields.empty() || f.subfields[0].items.empty())
    //             continue;

    //         // Check the last 3 digits of the tag
    //         if (tag.size() >= 3) {
    //             std::string suffix = tag.substr(tag.size() - 3);
    //             if (suffix == "006" || suffix == ".06")
    //                 w =
    //                 std::stoul(DecodeText(f.subfields[0].items[0].bytes));
    //             else if (suffix == "007" || suffix == ".07")
    //                 h =
    //                 std::stoul(DecodeText(f.subfields[0].items[0].bytes));
    //             else if (suffix == "011" || suffix == ".11")
    //                 compression = DecodeText(f.subfields[0].items[0].bytes);
    //         }
    //     }

    //     // 2. The "Magic Byte" Scanner
    //     // We scan the raw_field for the actual start of the image stream
    //     const uint8_t* data = field->raw_field.data();
    //     size_t size = field->raw_field.size();
    //     const uint8_t* imgStart = nullptr;

    //     for (size_t i = 0; i < size - 1; ++i) {
    //         // WSQ Detection (FF A0)
    //         if (data[i] == 0xFF && data[i + 1] == 0xA0) {
    //             imgStart = &data[i];
    //             break;
    //         }
    //         // JPEG Detection (FF D8)
    //         if (data[i] == 0xFF && data[i + 1] == 0xD8) {
    //             imgStart = &data[i];
    //             break;
    //         }
    //         // JP2K Detection (00 00 00 0C)
    //         if (i < size - 4 && data[i] == 0x00 && data[i + 1] == 0x00 &&
    //             data[i + 2] == 0x00 && data[i + 3] == 0x0C) {
    //             imgStart = &data[i];
    //             break;
    //         }
    //     }

    //     // 3. UI Feedback
    //     if (imgStart) {
    //         size_t offset = imgStart - data;
    //         ImGui::Text("Detected %s at offset %zu", compression.c_str(),
    //                     offset);

    //         // This is where you'd call:
    //         // MyDecompressor::Decode(imgStart, size - offset, compression);
    //     } else {
    //         ImGui::TextDisabled(
    //             "No recognizable image header found in raw_field.");
    //     }
    // }
    // inline void RenderBinaryFieldEditor(nnist::Record* record,
    //                                     nnist::Field* field) {
    //     if (!field || field->raw_field.empty()) {
    //         ImGui::TextDisabled("No raw data in field.");
    //         return;
    //     }

    //     // 1. Locate the data start (Skip "14.999:")
    //     // We look for the first colon ':' or the first byte > 0x1F after the
    //     // tag
    //     size_t dataOffset = 0;
    //     for (size_t i = 0; i < field->raw_field.size(); ++i) {
    //         if (field->raw_field[i] == ':') {
    //             dataOffset = i + 1;
    //             break;
    //         }
    //     }

    //     const uint8_t* blobPtr = field->raw_field.data() + dataOffset;
    //     size_t blobSize = field->raw_field.size() - dataOffset;

    //     if (blobSize == 0) {
    //         ImGui::TextDisabled("Field contains tag but no binary payload.");
    //         return;
    //     }

    //     // 2. Identify the Content-Type
    //     // WSQ starts with 0xFF 0xA0
    //     // JPEG starts with 0xFF 0xD8
    //     bool isJP2K = (blobSize > 4 && blobPtr[0] == 0x00 &&
    //                    blobPtr[1] == 0x00 && blobPtr[2] == 0x00);
    //     bool isWSQ = (blobSize > 2 && blobPtr[0] == 0xFF && blobPtr[1] ==
    //     0xA0); bool isJPG = (blobSize > 2 && blobPtr[0] == 0xFF && blobPtr[1]
    //     == 0xD8);

    //     ImGui::Text("Raw Buffer: %zu bytes", blobSize);

    //     if (isJP2K) {
    //         ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f),
    //                            "Format: JPEG 2000 (JP2)");
    //         // Use OpenJPEG to decompress imgData into a raw buffer
    //         // Then: texCache[field] = std::make_unique<NMTK::Texture>(w, h,
    //         // decompressedBuffer);
    //     }
    //     if (isWSQ) {
    //         ImGui::TextColored(ImVec4(0, 1, 0, 1), "Detected WSQ
    //         Compression");
    //         // To render: Needs wsq_decode() -> NMTK::Texture
    //     } else if (isJPG) {
    //         ImGui::TextColored(ImVec4(0, 1, 0, 1), "Detected JPEG Image");
    //         // To render: Needs stbi_load_from_memory() -> NMTK::Texture
    //     } else {
    //         ImGui::TextDisabled(
    //             "Unknown Binary Format (First bytes: %02X %02X)", blobPtr[0],
    //             blobPtr[1]);
    //     }
    // }
    // inline void RenderBinaryFieldEditor(nnist::Record* record,
    //                                     nnist::Field* field) {
    //     if (record == nullptr || field == nullptr) {
    //         ImGui::TextDisabled("Invalid data found, can't render.");
    //         return;
    //     }
    //     static std::unordered_map<nnist::Field*,
    //     std::unique_ptr<NMTK::Texture>>
    //         texCache;

    //     // Get dimensions from sibling field
    //     uint32_t width = 0;
    //     uint32_t height = 0;
    //     for (auto& f : record->fields) {
    //         std::string tag = DecodeText(f.tag);
    //         if (tag == "6") {
    //             width =
    //             std::stoul(DecodeText(f.subfields[0].items[0].bytes));
    //         }
    //         if (tag == "7") {
    //             height =
    //             std::stoul(DecodeText(f.subfields[0].items[0].bytes));
    //         }

    //         // Extract raw bytes from binary field
    //         const std::vector<uint8_t>& pixelData =
    //             field->subfields[0].items[0].bytes;
    //         if (texCache.find(field) == texCache.end() && !pixelData.empty()
    //         &&
    //             width > 0) {
    //             texCache[field] = std::make_unique<NMTK::Texture>(
    //                 width, height, pixelData.data());
    //         }
    //         // Render
    //         if (texCache.count(field)) {
    //             ImGui::Text("Binary Image Data (%zu bytes)",
    //             pixelData.size());
    //             ImGui::Image(texCache[field]->GetImGuiID(), ImVec2(256,
    //             256));

    //             if (ImGui::Button("Replace Image...")) {
    //                 // Trigger a NISTMutateCommand to inject new bytes into
    //                 // field->subfields[0].items[0].bytes
    //             }
    //         } else {
    //             ImGui::TextDisabled(
    //                 "Binary field detected, but dimensions (006/007) are "
    //                 "missing or invalid.");
    //         }
    //     }
    // }
}  // namespace OpenEFT
