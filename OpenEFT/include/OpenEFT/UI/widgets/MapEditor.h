#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/rules/Rules.h>

#include <functional>
#include <string>
#include <vector>
namespace OpenEFT {
    template <typename T>
    class MapEditor {
      public:
        T* selected = nullptr;

        std::string selected_key;
        std::string context_key;
        std::string pending_delete;

        T add_buffer{};
        std::string add_key_buffer;

        void Draw(
            std::unordered_map<std::string, T>* map, const char* popupEdit,
            const char* popupAdd, const char* addLabel,
            std::function<std::string(const std::string&, const T&)> getLabel,
            std::function<void(T&)> drawEditor,
            std::function<void(T&)> drawAdder,
            std::function<void(const std::string&, T&)> onAdd,
            std::function<void()> onSelect = nullptr,
            std::function<void()> onDelete = nullptr) {
            if (!map) {
                ImGui::TextDisabled("Unavailable.");
                return;
            }

            ImGui::PushID(this);
            auto& items = *map;

            // --- validate selection ---
            if (!selected_key.empty()) {
                auto it = items.find(selected_key);
                if (it != items.end()) {
                    selected = &it->second;
                } else {
                    selected_key.clear();
                    selected = nullptr;
                }
            } else {
                selected = nullptr;
            }

            // --- (optional) sorted iteration ---
            std::vector<std::pair<std::string, T*>> sorted;
            for (auto& [k, v] : items)
                sorted.emplace_back(k, &v);

            std::sort(sorted.begin(), sorted.end(),
                      [](auto& a, auto& b) { return a.first < b.first; });

            // --- list ---
            for (auto& [key, value] : sorted) {
                bool is_selected = (selected_key == key);

                std::string label = getLabel(key, *value);

                if (ImGui::Selectable(label.c_str(), is_selected)) {
                    selected_key = key;
                    selected = value;
                    if (onSelect)
                        onSelect();
                }

                if (ImGui::IsItemHovered() &&
                    ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
                    context_key = key;
                    ImGui::OpenPopup(popupEdit);
                }
            }

            // --- edit popup ---
            if (ImGui::BeginPopup(popupEdit)) {
                if (!context_key.empty() && items.count(context_key)) {
                    drawEditor(items[context_key]);

                    if (ImGui::Button("Delete")) {
                        pending_delete = context_key;
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Cancel")) {
                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::EndPopup();
            }

            // --- delete ---
            if (!pending_delete.empty() && items.count(pending_delete)) {
                if (selected_key == pending_delete) {
                    selected_key.clear();
                    selected = nullptr;
                }

                items.erase(pending_delete);

                if (onDelete)
                    onDelete();

                pending_delete.clear();
            }

            // --- add ---
            if (ImGui::Button(addLabel)) {
                ImGui::OpenPopup(popupAdd);
            }

            if (ImGui::BeginPopup(popupAdd)) {
                ImGui::InputText("Key", &add_key_buffer);

                drawAdder(add_buffer);

                if (ImGui::Button("Add")) {
                    if (!add_key_buffer.empty()) {
                        onAdd(add_key_buffer, add_buffer);
                    }
                    add_buffer = {};
                    add_key_buffer.clear();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    add_buffer = {};
                    add_key_buffer.clear();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::PopID();
        }
    };
}  // namespace OpenEFT
