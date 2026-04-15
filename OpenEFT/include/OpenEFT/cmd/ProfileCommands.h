#pragma once
#include <NMTK/cmd/ICommand.h>
#include <OpenEFT/core/Project.h>

#include <filesystem>
#include <optional>
#include <string>

namespace OpenEFT {
    class CreateProfileCommand : public NMTK::ICommand {
      public:
        CreateProfileCommand(std::string name, std::filesystem::path dir)
            : m_Name(std::move(name)), m_Dir(std::move(dir)) {}

        const std::string Name() const override { return "Create Profile"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& profile = proj.Profiles().CreateProfile(m_Name, m_Dir);
            m_Created = true;
        }
        void Undo(NMTK::IProject& project) override {
            if (!m_Created)
                return;

            auto& proj = static_cast<Project&>(project);
            proj.Profiles().DeleteProfile(m_Name);
        }

      private:
        std::string m_Name;
        std::filesystem::path m_Dir;
        bool m_Created = false;
    };

    class SetActiveProfileCommand : public NMTK::ICommand {
      public:
        SetActiveProfileCommand(std::string name) : m_New(std::move(name)) {}

        const std::string Name() const override { return "Set Active Profile"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Profiles();

            if (auto* active = mgr.GetActive()) {
                m_Previous = active->GetName();
            }

            mgr.SetActive(m_New);
        }

        void Undo(NMTK::IProject& project) override {
            if (m_Previous.empty())
                return;

            auto& proj = static_cast<Project&>(project);
            proj.Profiles().SetActive(m_Previous);
        }

      private:
        std::string m_New;
        std::string m_Previous;
    };

    class DeleteProfileCommand : public NMTK::ICommand {
      public:
        DeleteProfileCommand(std::string name) : m_Name(std::move(name)) {}

        const std::string Name() const override { return "Delete Profile"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Profiles();

            auto* p = mgr.GetActive();  // optional: capture active
            if (p && p->GetName() == m_Name) {
                m_WasActive = true;
            }

            CORE_WARN("DELETE COMMAND: {}", m_Name);

            // snapshot for undo
            m_Backup = mgr.ExportProfile(m_Name);

            mgr.DeleteProfile(m_Name);
        }

        void Undo(NMTK::IProject& project) override {
            if (!m_Backup)
                return;

            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Profiles();

            mgr.ImportProfile(*m_Backup);

            if (m_WasActive) {
                mgr.SetActive(m_Backup->GetName());
            }
        }

      private:
        std::string m_Name;
        std::optional<Profile> m_Backup;
        bool m_WasActive = false;
    };

    class RenameProfileCommand : public NMTK::ICommand {
      public:
        RenameProfileCommand(std::string oldName, std::string newName)
            : m_Old(std::move(oldName)), m_New(std::move(newName)) {}

        const std::string Name() const override { return "Rename Profile"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Profiles();

            mgr.RenameProfile(m_Old, m_New);
        }

        void Undo(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Profiles();

            mgr.RenameProfile(m_New, m_Old);
        }

      private:
        std::string m_Old, m_New;
    };

    class LoadProfileCommand : public NMTK::ICommand {
      public:
        LoadProfileCommand(std::filesystem::path path)
            : m_Path(std::move(path)) {}

        const std::string Name() const override { return "Load Profile"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Profiles();
            mgr.LoadProfile(m_Path);
        }

        void Undo(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Profiles();
            mgr.UnloadProfile(m_Path);
        }

      private:
        std::filesystem::path m_Path;
    };

    class UnloadProfileCommand : public NMTK::ICommand {
      public:
        UnloadProfileCommand(std::string name) : m_Name(std::move(name)) {}

        const std::string Name() const override { return "Unload Profile"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Profiles();
            // Get path from profile
            auto prof = mgr.ExportProfile(m_Name);
            if (!prof.has_value()) {
                CORE_ERROR("Failed to find profile: {}", m_Name);
                return;
            }
            m_Path = prof.value().GetPath();
            mgr.UnloadProfile(m_Name);
        }
        void Undo(NMTK::IProject& project) override {
            if (m_Path.string().empty()) {
                CORE_WARN("No profile to load.");
                return;
            }
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Profiles();
            mgr.LoadProfile(m_Path);
        }

      private:
        std::string m_Name;
        std::filesystem::path m_Path;
    };

    class SaveActiveProfileCommand : public NMTK::ICommand {
      public:
        SaveActiveProfileCommand() {}

        const std::string Name() const override {
            return "Save Active Profile";
        }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Profiles();
            mgr.SaveActive();
        }
        void Undo(NMTK::IProject& project) override {
            CORE_INFO("Can't undo saves.");
        }
    };

    class SaveActiveProfileAsCommand : public NMTK::ICommand {
      public:
        SaveActiveProfileAsCommand(std::filesystem::path path)
            : m_Path(std::move(path)) {}

        const std::string Name() const override {
            return "Save Active Profile As";
        }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Profiles();
            auto* p = mgr.GetActive();
            if (!p) {
                CORE_INFO("No active profile.");
                return;
            }
            m_OldPath = p->GetPath();
            p->SetPath(m_Path);
            mgr.SaveActive();
        }

        void Undo(NMTK::IProject& project) override {
            CORE_INFO("Can't undo saves.");
        }

      private:
        std::filesystem::path m_OldPath;
        std::filesystem::path m_Path;
    };

    class SaveAllProfilesCommand : public NMTK::ICommand {
      public:
        SaveAllProfilesCommand() {}

        const std::string Name() const override { return "Save All Profiles"; }

        void Execute(NMTK::IProject& project) override {
            auto& proj = static_cast<Project&>(project);
            auto& mgr = proj.Profiles();
            mgr.SaveAll();
        }

        void Undo(NMTK::IProject& project) override {
            CORE_INFO("Can't undo saves.");
        }
    };
}  // namespace OpenEFT
