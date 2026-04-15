#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/cmd/Commands.h>
#include <OpenEFT/core/Project.h>

namespace OpenEFT {
    struct FileExplorer : public NMTK::IPanel {
        FileExplorer(std::shared_ptr<NMTK::AppContext> ctx)
            : NMTK::IPanel(std::move(ctx)) {
            m_Name = "File Explorer";
        }

        void Render() override {
            ImGui::Begin(m_Name.c_str(), &m_IsOpen);
            auto& proj = m_AppContext->project();
            auto& project = static_cast<Project&>(proj);
            auto& fmgr = project.Files();
            for (auto& [path, files] : fmgr.All()) {
                if (ImGui::Selectable(path.filename().string().c_str(),
                                      fmgr.IsActive(path))) {
                    m_AppContext->cmdManager().Run(
                        std::make_unique<SetActiveFileCommand>(path),
                        m_AppContext->project());
                }
            }

            ImGui::End();
        }
    };
}  // namespace OpenEFT
