#pragma once

#include <NMTK/cmd/ICommand.h>
#include <OpenEFT/core/Project.h>
#include <OpenEFT/validator/Data.h>

#include <vector>
#include <string>
#include <filesystem>

namespace OpenEFT {
    class RunValidationCommand : public NMTK::ICommand {
      public:
        RunValidationCommand() {}

        const std::string Name() const override { return "Run Validation"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& vmgr = proj.Validator();
            vmgr.ValidateActiveFile();
        }

        void Undo(NMTK::IProject& project) override {
            CORE_WARN("Can't undo validation.");
        }
    };

    class ClearValidationResultsCommand : public NMTK::ICommand {
      public:
        ClearValidationResultsCommand() {}
        const std::string Name() const override {
            return "Clear Validation Results";
        }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& vmgr = proj.Validator();
            m_Diagnostics = vmgr.GetDiagnostics();
            vmgr.ClearDiagnostics();
        }

        void Undo(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& vmgr = proj.Validator();
            vmgr.SetDiagnostics(m_Diagnostics);
        }

      private:
        std::vector<Diagnostic> m_Diagnostics;
    };

    class ExportValidationResultsCommand : public NMTK::ICommand {
        public:
            ExportValidationResultsCommand(const std::filesystem::path& p)
                : m_Path(p) {}
            const std::string Name() const override {
                return "Export Validation Results";
            }

            void Execute(NMTK::IProject& project) override {
                auto& proj = static_cast<Project&>(project);
                auto& vmgr = proj.Validator();
                vmgr.ExportValidationResults(m_Path);
            } 
            void Undo(NMTK::IProject& project) override {
                CORE_INFO("Can't undo export validation results.");
            }
        private:
            std::filesystem::path m_Path;
    };
}  // namespace OpenEFT
