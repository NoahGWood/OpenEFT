// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/OpenEFT.h>
#include <tinyfiledialogs.h>

#include <cstring>
#include <string>
#include <vector>

namespace OpenEFT {

    // Example Menu
    struct SettingsMenu : public NMTK::IMenu {
        SettingsMenu(std::shared_ptr<NMTK::AppContext> ctx)
            : NMTK::IMenu(std::move(ctx)) {
            m_Name = "Settings";
        }
        void Render() override {
            if (ImGui::BeginMenu(m_Name.c_str())) {
                auto& proj = m_AppContext->project();
                auto& project = static_cast<Project&>(proj);
                auto& settings = project.Settings();

                if (ImGui::BeginMenu("Validation")) {
                    ImGui::MenuItem("Strict Enums", nullptr,
                                    &settings.strict_enums);
                    ImGui::MenuItem("Strict Occurrence", nullptr,
                                    &settings.strict_occurrence);
                    ImGui::MenuItem("Allow Unknown Fields", nullptr,
                                    &settings.allow_unknown_fields);
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("UI")) {
                    ImGui::MenuItem("Show Hex View", nullptr,
                                    &settings.show_hex_view);
                    ImGui::MenuItem("Highlight Errors", nullptr,
                                    &settings.highlight_errors);
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
        }
    };

}  // namespace OpenEFT
