// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#include <OpenEFT/UI/panels/FileHierarchyPanel.h>

namespace OpenEFT {
    void FileHierarchyPanel::Render() {
        ImGui::Begin(m_Name.c_str(), &m_IsOpen);
        auto& project = static_cast<Project&>(m_AppContext->project());
        auto& fm = project.Files();

        if (!fm.HasActive()) {
            ImGui::TextDisabled("No file selected.");
            ImGui::End();
            return;
        }

        auto& active = fm.GetActive();
        auto& currentSel = fm.GetSelection();

        for (size_t rIdx = 0; rIdx < active.records.size(); ++rIdx) {
            auto& record = active.records[rIdx];
            std::string rLabel = "Record " + std::to_string(record.type) +
                                 " [IDC: " + std::to_string(record.idc) + "]";

            ImGui::PushID(static_cast<int>(rIdx));

            // We use TreeNodeEx for records too, so we can detect clicks on the
            // record itself if needed
            bool recordOpen = ImGui::CollapsingHeader(rLabel.c_str());

            if (recordOpen) {
                if (record.is_binary_record) {
                    // ImGui::Text("Binary Data: %zu bytes",
                    //                   record.raw_record.size());
                    bool isSelected = currentSel.recordId == (int)rIdx;
                    std::string label = "Binary Data: " + std::to_string(record.raw_record.size()) + " bytes";
                    if(ImGui::Selectable(label.c_str(), isSelected)){
                        fm.SetSelection({&record, nullptr, (int)rIdx, -1});
                    }
                } else {
                    for (size_t fIdx = 0; fIdx < record.fields.size(); ++fIdx) {
                        auto& field = record.fields[fIdx];
                        std::string fTag = DecodeText(field.tag);

                        // Highlighting logic: check if this specific field is
                        // the one selected
                        bool isSelected = (currentSel.recordId == (int)rIdx &&
                                           currentSel.fieldIdx == (int)fIdx);

                        ImGui::PushID(static_cast<int>(fIdx));

                        // Use a Selectable wrapper for the tree node
                        ImGuiTreeNodeFlags flags =
                            ImGuiTreeNodeFlags_OpenOnArrow |
                            ImGuiTreeNodeFlags_SpanAvailWidth;
                        if (isSelected)
                            flags |= ImGuiTreeNodeFlags_Selected;

                        bool nodeOpen = ImGui::TreeNodeEx(fTag.c_str(), flags);

                        // If the user clicks the label (not the arrow), update
                        // selection
                        if (ImGui::IsItemClicked() &&
                            !ImGui::IsItemToggledOpen()) {
                            fm.SetSelection(
                                {&record, &field, (int)rIdx, (int)fIdx});
                        }

                        if (nodeOpen) {
                            for (size_t sIdx = 0; sIdx < field.subfields.size();
                                 ++sIdx) {
                                if (ImGui::TreeNode((void*)(intptr_t)sIdx,
                                                    "Subfield %zu", sIdx + 1)) {
                                    for (size_t iIdx = 0;
                                         iIdx <
                                         field.subfields[sIdx].items.size();
                                         ++iIdx) {
                                        std::string data =
                                            DecodeText(field.subfields[sIdx]
                                                           .items[iIdx]
                                                           .bytes);
                                        ImGui::BulletText("[%zu]: %s", iIdx,
                                                          data.empty()
                                                              ? "<empty>"
                                                              : data.c_str());
                                    }
                                    ImGui::TreePop();
                                }
                            }
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                }
            }
            ImGui::PopID();
        }
        ImGui::End();
    }
}  // namespace OpenEFT
