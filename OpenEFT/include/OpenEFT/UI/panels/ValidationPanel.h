// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/core/Project.h>
#include <OpenEFT/validator/Helpers.h>

namespace OpenEFT {
    struct ValidationPanel : public NMTK::IPanel {
        ValidationPanel(std::shared_ptr<NMTK::AppContext> ctx)
            : NMTK::IPanel(std::move(ctx)) {
            m_Name = "Validation";
        }
        void Render() override {
            ImGui::Begin(m_Name.c_str(), &m_IsOpen);
            auto& proj = m_AppContext->project();
            auto& project = static_cast<Project&>(proj);
            auto& vmgr = project.Validator();

            auto& diagnostics = vmgr.GetDiagnostics();

            if (diagnostics.empty()) {
                ImGui::TextDisabled("No validation issues.");
                ImGui::End();
                return;
            }

            if (ImGui::BeginTable(
                    "DiagnosticsTable", 4,
                    ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
                        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY)) {
                ImGui::TableSetupColumn(
                    "Severity", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn(
                    "Record", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn(
                    "Field", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("Message",
                                        ImGuiTableColumnFlags_WidthStretch);

                ImGui::TableHeadersRow();

                for (const auto& dx : diagnostics) {
                    ImGui::TableNextRow();

                    // --- Severity ---
                    ImGui::TableSetColumnIndex(0);
                    ImVec4 color = SeverityColor(dx.severity);
                    ImGui::TextColored(color, "%s", SeverityText(dx.severity));

                    // --- Record ---
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("Type-%u", dx.record_type);

                    // --- Field ---
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%u.%03u", dx.record_type, dx.field_id);

                    // --- Message ---
                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextWrapped("%s", dx.message.c_str());
                }

                ImGui::EndTable();
            }
            ImGui::End();
        }
    };
}  // namespace OpenEFT
