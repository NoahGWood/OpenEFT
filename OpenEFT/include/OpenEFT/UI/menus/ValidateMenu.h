#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/OpenEFT.h>
#include <tinyfiledialogs.h>

#include <cstring>
#include <string>
#include <vector>

namespace OpenEFT {

    // Example Menu
    struct ValidateMenu : public NMTK::IMenu {
        ValidateMenu(std::shared_ptr<NMTK::AppContext> ctx)
            : NMTK::IMenu(std::move(ctx)) {
            m_Name = "Validate";
        }
        void Render() override {
            if (ImGui::BeginMenu(m_Name.c_str())) {
                auto& proj = m_AppContext->project();
                auto& project = static_cast<Project&>(proj);
                auto& settings = project.Settings();

                if (ImGui::MenuItem("Run Validation")) {
                    m_AppContext->cmdManager().Run(
                        std::make_unique<RunValidationCommand>(),
                        m_AppContext->project());
                }
                // ToDo
                // ImGui::MenuItem("Auto Validate", nullptr,
                // &settings.auto_validate);

                if (ImGui::MenuItem("Clear Results")) {
                    m_AppContext->cmdManager().Run(
                        std::make_unique<ClearValidationResultsCommand>(),
                        m_AppContext->project());
                }

                if (ImGui::MenuItem("Export Results...")) {
                    ExportResults();
                }
                ImGui::EndMenu();
            }
        }

        void ExportResults() {
            const char* patterns[1] = {"*.csv"};

            char* file = tinyfd_saveFileDialog("Export Validation Results",
                                               "./validation.csv", 1, patterns,
                                               "Text File");

            if (!file)
                return;

            m_AppContext->cmdManager().Run(
                std::make_unique<ExportValidationResultsCommand>(file),
                m_AppContext->project());
        }
    };
}  // namespace OpenEFT
