#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/rules/Rules.h>

#include <functional>
#include <string>
#include <vector>
namespace OpenEFT {

    template <typename T>
    class ListEditor {
      public:
        T* selected = nullptr;

        int context_index = -1;
        int pending_delete = -1;

        T add_buffer{};

        void Draw(std::vector<T>* list, const char* popupEdit,
                  const char* popupAdd, const char* addLabel,
                  std::function<std::string(const T&)> getLabel,
                  std::function<void(T&)> drawEditor,
                  std::function<void(T&)> drawAdder,
                  std::function<void(T&)> onAdd,
                  std::function<void()> onSelect = nullptr,
                  std::function<void()> onDelete = nullptr,
                  bool increment_ids = false) {
            if (!list) {
                ImGui::TextDisabled("Unavailable.");
                return;
            }
            ImGui::PushID(this);

            auto& items = *list;

            // --- validate selection ---
            if (selected) {
                bool valid = false;
                for (auto& i : items) {
                    if (&i == selected) {
                        valid = true;
                        break;
                    }
                }
                if (!valid)
                    selected = nullptr;
            }

            // --- list ---
            for (int i = 0; i < items.size(); ++i) {
                bool is_selected = (selected == &items[i]);

                std::string label = getLabel(items[i]);
                if (increment_ids) {
                    label += std::to_string(i+1);
                }
                if (ImGui::Selectable(label.c_str(), is_selected)) {
                    selected = &items[i];
                    if (onSelect)
                        onSelect();
                }
                if (ImGui::IsItemHovered() &&
                    ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
                    context_index = i;
                    ImGui::OpenPopup(popupEdit);
                }
            }

            // --- edit popup ---
            if (ImGui::BeginPopup(popupEdit)) {
                if (context_index >= 0 && context_index < items.size()) {
                    drawEditor(items[context_index]);

                    if (ImGui::Button("Delete")) {
                        pending_delete = context_index;
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
            if (pending_delete >= 0 && pending_delete < items.size()) {
                if (selected == &items[pending_delete])
                    selected = nullptr;
                items.erase(items.begin() + pending_delete);
                if(onDelete){
                    onDelete();
                }
                pending_delete = -1;
            }

            // --- add ---
            if (ImGui::Button(addLabel)) {
                ImGui::OpenPopup(popupAdd);
            }

            if (ImGui::BeginPopup(popupAdd)) {
                drawAdder(add_buffer);

                if (ImGui::Button("Add")) {
                    onAdd(add_buffer);
                    add_buffer = {};
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    add_buffer = {};
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::PopID();
        }
    };
}  // namespace OpenEFT
