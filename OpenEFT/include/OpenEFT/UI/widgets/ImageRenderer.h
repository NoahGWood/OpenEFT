#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/image/Image.h>
#include <nnist/nnist.h>
namespace OpenEFT {
    class ImageRenderer {
      public:
    };

    // inline void RenderImage(const Image& img) {
    //     if (!img.IsValid()) {
    //         ImGui::Text("Invalid image");
    //         return;
    //     }

    //     // TEMP: scale down if large
    //     float maxSize = 300.0f;
    //     float scale =
    //         std::min(maxSize / img.info.width, maxSize / img.info.height);

    //     ImVec2 size(img.info.width * scale, img.info.height * scale);

    //     // You’ll replace this with your GPU texture system
    //     ImGui::Text("Preview not yet uploaded to GPU");
    //     ImGui::Dummy(size);
    // }

    // inline void RenderBinaryWithInspector(
    //     nnist::Record& record, nnist::Field& field, ImageRegistry& registry,
    //     std::shared_ptr<NMTK::AppContext> ctx) {
    //     static std::unordered_map<int, std::unique_ptr<NMTK::Texture>> cache;

    //     const ImageInspector* inspector = registry.Resolve(record, field);
    //     if (!inspector) {
    //         ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1),
    //                            "No inspector available.");
    //         return;
    //     }

    //     Image img;
    //     if (!inspector->Decode(record, field, img, ctx) || !img.IsValid()) {
    //         ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1),
    //                            "Failed to decode image.");
    //         return;
    //     }

    //     // ---- Cache key (use IDC if available, fallback to pointer) ----
    //     int key = record.idc;

    //     if (!cache.count(key)) {
    //         // ⚠️ RAW handling only (like your old code)
    //         if (img.info.channels == 1) {
    //             cache[key] = std::make_unique<NMTK::Texture>(
    //                 img.info.width, img.info.height, img.pixels.data());
    //         } else if (img.info.channels == 3) {
    //             // If your texture supports RGB directly
    //             cache[key] = std::make_unique<NMTK::Texture>(
    //                 img.info.width, img.info.height, img.pixels.data());
    //         } else {
    //             ImGui::Text("Unsupported channel count: %d",
    //             img.info.channels); return;
    //         }
    //     }

    //     // ---- Render ----
    //     ImGui::Text("Image (%d x %d)", img.info.width, img.info.height);

    //     float availWidth = ImGui::GetContentRegionAvail().x;
    //     float displayHeight =
    //         availWidth * ((float)img.info.height / (float)img.info.width);

    //     ImGui::Image(cache[key]->GetImGuiID(),
    //                  ImVec2(availWidth, displayHeight));
    // }

    inline void RenderTaggedRecord(nnist::Record& record, nnist::Field& field,
                                   ImageRegistry& registry,
                                   std::shared_ptr<NMTK::AppContext> ctx) {
        struct CacheEntry {
            std::vector<uint8_t> pixels;
            std::unique_ptr<NMTK::Texture> texture;
            int width = 0;
            int height = 0;
        };

        static std::unordered_map<uint64_t, CacheEntry> cache;

        // ---- Resolve inspector ----
        const ImageInspector* inspector = registry.Resolve(record, field);
        if (!inspector) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "No inspector available.");
            return;
        }

        // ---- Decode ----
        Image img;
        if (!inspector->Decode(record, field, img, ctx) || !img.IsValid()) {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Decode failed.");
            if(!img.info.error.empty()){
                ImGui::TextColored(ImVec4(1,0,0,1), img.info.error.c_str());
            }
            return;
        }

        // ---- Cache key (stable + unique) ----
        uint64_t key =
            ((uint64_t)record.idc << 32) ^ ((uint64_t)record.type << 32);

        auto& entry = cache[key];

        // ---- Rebuild texture if needed ----
        if (!entry.texture || entry.width != img.info.width ||
            entry.height != img.info.height) {
            entry.width = img.info.width;
            entry.height = img.info.height;

            // ---- Normalize to RGBA ----
            // entry.pixels.clear();
            // entry.pixels.reserve(img.info.width * img.info.height * 4);
            // 🔥 CRITICAL: copy pixels so memory stays valid
            entry.pixels = img.pixels;

            entry.texture = std::make_unique<NMTK::Texture>(
                img.info.width, img.info.height, entry.pixels.data(),
                img.info.channels);
            // if (img.info.channels == 1) {
            //     // grayscale → RGBA
            //     for (size_t i = 0; i < img.pixels.size(); ++i) {
            //         uint8_t v = img.pixels[i];
            //         entry.pixels.push_back(v);
            //         entry.pixels.push_back(v);
            //         entry.pixels.push_back(v);
            //         entry.pixels.push_back(255);
            //     }
            // } else if (img.info.channels == 3) {
            //     // RGB → RGBA
            //     for (size_t i = 0; i < img.pixels.size(); i += 3) {
            //         entry.pixels.push_back(img.pixels[i + 0]);
            //         entry.pixels.push_back(img.pixels[i + 1]);
            //         entry.pixels.push_back(img.pixels[i + 2]);
            //         entry.pixels.push_back(255);
            //     }
            // } else {
            //     ImGui::Text("Unsupported channel count: %d",
            //     img.info.channels); return;
            // }
            // entry.texture = std::make_unique<NMTK::Texture>(
            //     img.info.width, img.info.height, entry.pixels.data(),
            //     4  // RGBA
            // );
        }

        // ---- Render ----
        ImGui::Text("Type-%d (%d x %d)", record.type, img.info.width,
                    img.info.height);

        float availWidth = ImGui::GetContentRegionAvail().x;
        float displayHeight =
            availWidth * ((float)img.info.height / (float)img.info.width);

        ImGui::Image(entry.texture->GetImGuiID(),
                     ImVec2(availWidth, displayHeight));
    }
    
    inline void RenderBinaryRecord(nnist::Record& record,
                                   ImageRegistry& registry,
                                   std::shared_ptr<NMTK::AppContext> ctx) {
        nnist::Field field{};  // Empty field
        RenderTaggedRecord(record, field, registry, ctx);
    }
}  // namespace OpenEFT
