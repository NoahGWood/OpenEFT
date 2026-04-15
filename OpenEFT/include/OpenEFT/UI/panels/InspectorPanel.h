#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/OpenEFT.h>
#include <OpenEFT/UI/widgets/ImageRenderer.h>

namespace OpenEFT {
    struct InspectorPanel : public NMTK::IPanel {
        InspectorPanel(std::shared_ptr<NMTK::AppContext> ctx)
            : NMTK::IPanel(std::move(ctx)) {
            m_Name = "Inspector";
        }

        void Render() override {
            ImGui::Begin(m_Name.c_str(), &m_IsOpen);

            auto& project = static_cast<Project&>(m_AppContext->project());
            auto& fm = project.Files();

            if (!fm.HasActive()) {
                ImGui::TextDisabled("Select a field in the Hierarchy to edit.");
                ImGui::End();
                return;
            }

            auto& selection = fm.GetSelection();
            auto* record = selection.record;
            if (record == nullptr) {
                ImGui::TextDisabled("No record selected.");
                ImGui::End();
                return;
            }
            auto& registry = project.Images();
            if (record->is_binary_record) {
                // RenderBinaryWithInspector(*record, registry,
                //                           m_AppContext);
                RenderBinaryRecord(*record, registry, m_AppContext);
                ImGui::End();
                return;
            }

            if (fm.GetSelection().field == nullptr) {
                ImGui::TextDisabled("Select a field in the Hierarchy to edit.");
                ImGui::End();
                return;
            }
            auto* field = selection.field;

            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Field Tag: %s",
                               DecodeText(field->tag).c_str());
            ImGui::Separator();

            // If field is a tagged image record, just draw it & exit
            if (field->is_binary_field) {
                // RenderBinaryFieldEditor(record, field);
                RenderTaggedRecord(*record, *field, registry, m_AppContext);
                ImGui::End();
                return;
            }
            // Iterate through subfields and items
            for (size_t sIdx = 0; sIdx < field->subfields.size(); ++sIdx) {
                auto& subfield = field->subfields[sIdx];

                if (ImGui::TreeNodeEx((void*)(intptr_t)sIdx,
                                      ImGuiTreeNodeFlags_DefaultOpen,
                                      "Subfield %zu", sIdx + 1)) {
                    for (size_t iIdx = 0; iIdx < subfield.items.size();
                         ++iIdx) {
                        auto& item = subfield.items[iIdx];

                        // Convert bytes to string for ImGui
                        std::string content = DecodeText(item.bytes);

                        // We use a callback-based InputText to handle
                        // std::string directly
                        char buf[1024];
                        strncpy(buf, content.c_str(), sizeof(buf));

                        std::string label = "Item " + std::to_string(iIdx + 1);
                        if (ImGui::InputText(label.c_str(), buf, sizeof(buf))) {
                            // Update raw bytes
                            std::string updated(buf);
                            item.bytes.assign(updated.begin(), updated.end());

                            // Logic for dirtying the file could go here
                            // fm.MarkActiveAsDirty();
                        }
                    }
                    ImGui::TreePop();
                }
            }

            // Optional: Add a 'Delete Field' or 'Add Subfield' button at the
            // bottom
            ImGui::Separator();
            if (ImGui::Button("Add Subfield")) {
                field->subfields.push_back(nnist::Subfield{});
            }

            ImGui::End();
        }
    };
}  // namespace OpenEFT
