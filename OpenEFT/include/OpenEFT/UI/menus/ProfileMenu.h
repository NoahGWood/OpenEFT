// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/OpenEFT.h>
#include <OpenEFT/UI/popups/AsyncOpenFileDialog.h>
#include <OpenEFT/UI/popups/AsyncSaveFileDialog.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace OpenEFT {

    // Example Menu
    struct ProfileMenu : public NMTK::IMenu {
        ProfileMenu(std::shared_ptr<NMTK::AppContext> ctx)
            : NMTK::IMenu(std::move(ctx)) {
            m_Name = "Profile";
        }

        AsyncOpenFileDialog m_OpenDialog;
        AsyncSaveFileDialog m_SaveDialog;

        void Render() override {
            m_OpenDialog.Pump();
            m_SaveDialog.Pump();

            if (ImGui::BeginMenu(m_Name.c_str())) {
                auto& proj = static_cast<Project&>(m_AppContext->project());
                auto& pm = proj.Profiles();

                if (ImGui::MenuItem("Load Profile")) {
                    Open();
                }
                ImGui::Separator();
                if (ImGui::BeginMenu("Active Profile")) {
                    std::vector<std::string> names;
                    for (auto& [name, _] : pm.All())
                        names.push_back(name);

                    std::sort(names.begin(), names.end());

                    for (auto& name : names) {
                        bool selected = pm.IsActive(name);
                        if (selected) {
                            if (ImGui::BeginMenu(name.c_str())) {
                                auto& all = pm.GetActive()->AllTransactions();
                                auto* active = pm.GetActiveTransaction();
                                for (auto& [name, tx] : all) {
                                    bool tx_selected = active == &tx;
                                    if (ImGui::MenuItem(name.c_str(), nullptr,
                                                        tx_selected)) {
                                        m_AppContext->cmdManager().Run(
                                            std::make_unique<
                                                SetActiveTransactionCommand>(
                                                name),
                                            proj);
                                    }
                                }
                                ImGui::EndMenu();
                            }
                        } else {
                            if (ImGui::MenuItem(name.c_str(), nullptr,
                                                selected)) {
                                m_AppContext->cmdManager().Run(
                                    std::make_unique<SetActiveProfileCommand>(
                                        name),
                                    m_AppContext->project());
                            }
                        }
                    }
                    if (ImGui::MenuItem("Save Active Profile")) {
                        Save();
                    }
                    if (ImGui::MenuItem("Save Active Profile As")) {
                        SaveAs();
                    }

                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Save All Profiles")) {
                    SaveAll();
                }
                ImGui::EndMenu();
            }
        }

        void Open() {
            FileDialogOptions opts;
            opts.title = "Open EBTS Profile";
            opts.default_path = "./profiles";
            opts.patterns = {"*.json"};
            opts.filter_description = "EBTS Profile";
            m_OpenDialog.Open(opts, [this](std::vector<std::string> files) {
                if (files.empty())
                    return;
                auto& proj = m_AppContext->project();
                for (auto& file : files) {
                    m_AppContext->cmdManager().Run(
                        std::make_unique<LoadProfileCommand>(file),
                        m_AppContext->project());
                }
            });
        }
        void Save() {
            m_AppContext->cmdManager().Run(
                std::make_unique<SaveActiveProfileCommand>(),
                m_AppContext->project());
        }
        void SaveAs() {
            auto& prj = static_cast<Project&>(m_AppContext->project());
            auto* active = prj.Profiles().GetActive();
            if (active == nullptr) {
                CORE_INFO("No active profile.");
                return;
            }
            SaveFileDialogOptions opts;
            opts.title = "Save EBTS Profile";
            opts.default_path = "profiles/" + active->GetName() + ".json";
            opts.patterns = {"*.json"};
            opts.filter_description = "EBTS Files";

            m_SaveDialog.Open(opts, [this](std::string path) {
                auto& proj = m_AppContext->project();
                auto& project = static_cast<Project&>(proj);
                m_AppContext->cmdManager().Run(
                    std::make_unique<SaveActiveProfileAsCommand>(path), proj);
            });
        }
        void SaveAll() {
            m_AppContext->cmdManager().Run(
                std::make_unique<SaveAllProfilesCommand>(),
                m_AppContext->project());
        }
    };

}  // namespace OpenEFT
