#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/OpenEFT.h>
#include <OpenEFT/UI/popups/AsyncOpenFileDialog.h>
#include <OpenEFT/UI/popups/AsyncSaveFileDialog.h>

#include <algorithm>
#include <filesystem>
#include <vector>

namespace OpenEFT {

    // Example Menu
    struct FileMenu : public NMTK::IMenu {
        FileMenu(std::shared_ptr<NMTK::AppContext> ctx)
            : NMTK::IMenu(std::move(ctx)) {
            m_Name = "File";
        }

        AsyncOpenFileDialog m_OpenDialog;
        AsyncSaveFileDialog m_SaveDialog;

        void Render() override {
            m_OpenDialog.Pump();
            m_SaveDialog.Pump();

            if (ImGui::BeginMenu(m_Name.c_str())) {
                auto& proj = m_AppContext->project();
                auto& project = static_cast<Project&>(proj);
                auto& fm = project.Files();

                if (ImGui::MenuItem("Open")) {
                    Open();
                }
                ImGui::Separator();
                if (ImGui::BeginMenu("Open Files")) {
                    std::vector<std::filesystem::path> paths;
                    for (auto& [path, _] : fm.All()) {
                        paths.push_back(path);
                    }
                    std::sort(paths.begin(), paths.end());

                    for (auto& path : paths) {
                        bool active = fm.IsActive(path);
                        if (ImGui::MenuItem(path.filename().string().c_str(),
                                            nullptr, active)) {
                            fm.SetActive(path);
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Save")) {
                    Save();
                }
                if (ImGui::MenuItem("Save As")) {
                    SaveAs();
                }
                if (ImGui::MenuItem("Save All")) {
                    SaveAll();
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Close File")) {
                    Close();
                }
                if (ImGui::MenuItem("Close All")) {
                    CloseAll();
                }
                ImGui::EndMenu();
            }
        }
        void Open() {
            FileDialogOptions opts;
            opts.title = "Open EFT File";
            opts.default_path = "./";
            opts.patterns = {"*.an2", "*.an2k", "*.eft"};
            opts.filter_description = "EFT Files";
            opts.allow_multi_select = true;

            m_OpenDialog.Open(opts, [this](std::vector<std::string> files) {
                if (files.empty())
                    return;

                auto& proj = m_AppContext->project();
                auto& project = static_cast<Project&>(proj);

                for (auto& path : files) {
                    project.Files().Open(path);
                }
            });
        }
        void Save() {
            m_AppContext->cmdManager().Run(
                std::make_unique<SaveActiveFileCommand>(),
                m_AppContext->project());
        }
        void SaveAs() {
            SaveFileDialogOptions opts;
            opts.title = "Save EFT File";
            opts.default_path = "output.eft";
            opts.patterns = {"*.eft"};
            opts.filter_description = "EFT Files";

            m_SaveDialog.Open(opts, [this](std::string path) {
                auto& proj = m_AppContext->project();
                auto& project = static_cast<Project&>(proj);
                m_AppContext->cmdManager().Run(
                    std::make_unique<SaveActiveFileAsCommand>(path),
                    m_AppContext->project());
            });
        }
        void SaveAll() {
            m_AppContext->cmdManager().Run(
                std::make_unique<SaveAllFilesCommand>(),
                m_AppContext->project());
        }
        void Close() {
            m_AppContext->cmdManager().Run(
                std::make_unique<CloseActiveFileCommand>(),
                m_AppContext->project());
        }
        void CloseAll() {
            m_AppContext->cmdManager().Run(
                std::make_unique<CloseAllFilesCommand>(),
                m_AppContext->project());
        }
    };

}  // namespace OpenEFT
