#pragma once
#include <NMTK/cmd/ICommand.h>
#include <OpenEFT/core/Project.h>
#include <nnist/nnist.h>

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

namespace OpenEFT {

    class OpenFileCommand : public NMTK::ICommand {
      public:
        OpenFileCommand(std::filesystem::path path) : m_Path(std::move(path)) {}

        const std::string Name() const override { return "Open File"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Files();
            mgr.Open(m_Path);
        }

        void Undo(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Files();
            mgr.Open(m_Path);
            mgr.Close(m_Path);
        }

      private:
        std::filesystem::path m_Path;
    };
    class CloseActiveFileCommand : public NMTK::ICommand {
      public:
        CloseActiveFileCommand() {}

        const std::string Name() const override { return "Close Active File"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Files();
            m_Path = mgr.GetActivePath();
            if (!std::filesystem::is_empty(m_Path)) {
                mgr.Close(mgr.GetActivePath());
            }
        }

        void Undo(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Files();
            mgr.Open(m_Path);
            if (std::filesystem::is_empty(m_Path)) {
                CORE_WARN("No file to undo closing.");
                return;
            }
            mgr.Open(m_Path);
        }

      private:
        std::filesystem::path m_Path;
    };
    class CloseAllFilesCommand : public NMTK::ICommand {
      public:
        CloseAllFilesCommand() {}

        const std::string Name() const override { return "Close All Files"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Files();
            m_Files = mgr.All();
            mgr.CloseAll();
        }

        void Undo(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Files();
            mgr.SetFiles(m_Files);
        }

      private:
        std::unordered_map<std::filesystem::path, nnist::File> m_Files;
    };

    class SaveActiveFileCommand : public NMTK::ICommand {
      public:
        SaveActiveFileCommand() {}

        const std::string Name() const override { return "Save Active File"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Files();
            mgr.SaveActive();
        }

        void Undo(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Files();
            CORE_WARN("Can't undo save.");
        }
    };
    class SaveActiveFileAsCommand : public NMTK::ICommand {
      public:
        SaveActiveFileAsCommand(std::filesystem::path path)
            : m_Path(std::move(path)) {}

        const std::string Name() const override {
            return "Save Active File As";
        }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Files();
            mgr.SaveActiveAs(m_Path);
        }

        void Undo(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Files();
            CORE_WARN("Can't undo save.");
        }

      private:
        std::filesystem::path m_Path;
    };

    class SaveAllFilesCommand : public NMTK::ICommand {
      public:
        SaveAllFilesCommand() {}

        const std::string Name() const override { return "Save All Files"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Files();
            mgr.SaveAll();
            mgr.SaveActive();
        }

        void Undo(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Files();
            CORE_WARN("Can't undo save.");
        }
    };
    class SetActiveFileCommand : public NMTK::ICommand {
      public:
        SetActiveFileCommand(std::filesystem::path path)
            : m_Path(std::move(path)) {}

        const std::string Name() const override { return "Set Active File"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Files();
            m_OldPath = mgr.GetActivePath();
            mgr.SetActive(m_Path);
        }

        void Undo(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Files();
            mgr.SetActive(m_OldPath);
            mgr.SetActive(m_OldPath);
        }

      private:
        std::filesystem::path m_Path;
        std::filesystem::path m_OldPath;
    };


}  // namespace OpenEFT
