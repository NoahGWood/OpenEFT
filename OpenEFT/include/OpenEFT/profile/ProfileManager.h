#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/profile/Profile.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace OpenEFT {
    class ProfileManager {
      public:
        ProfileManager(std::shared_ptr<NMTK::AppContext> ctx)
            : m_AppContext(std::move(ctx)) {}

        void LoadProfiles(const std::filesystem::path& dir);
        void LoadProfile(const std::filesystem::path& profile);
        void SaveAll();
        bool IsActive(const std::string& name) { return m_Active == name; }

        bool SetActive(const std::string& name) {
            auto it = m_Profiles.find(name);
            if (it == m_Profiles.end()) {
                CORE_ERROR("Profile {} not found.", name);
                return false;
            }
            m_Active = name;
            // Reset TX when switching profiles
            m_ActiveTransaction.clear();
            return true;
        }

        Profile* GetActive() {
            auto it = m_Profiles.find(m_Active);
            if (it == m_Profiles.end())
                return nullptr;
            return it->second.get();
        }

        const Profile* GetActive() const {
            auto it = m_Profiles.find(m_Active);
            if (it == m_Profiles.end())
                return nullptr;
            return it->second.get();
        }

        TransactionRule* GetActiveTransaction() {
            auto* profile = GetActive();
            if (!profile)
                return nullptr;

            auto& txs = profile->AllTransactions();

            auto it = txs.find(m_ActiveTransaction);
            if (it == txs.end()) {
                // fallback
                if (!txs.empty()) {
                    m_ActiveTransaction = txs.begin()->first;
                    return &txs.begin()->second;
                }
                return nullptr;
            }

            return &it->second;
        }
        
        bool IsActiveTransaction(const std::string& name) {
            return m_ActiveTransaction == name;
        }

        bool SetActiveTransaction(const std::string& name) {
            auto* profile = GetActive();
            if (!profile)
                return false;

            auto& txs = profile->AllTransactions();
            if (txs.find(name) == txs.end()) {
                CORE_WARN("Transaction {} not found in active profile.", name);
                return false;
            }

            m_ActiveTransaction = name;
            return true;
        }

        std::string GetActiveTransactionName() const {
            return m_ActiveTransaction;
        }

        void EnsureValidTransaction() {
            auto* profile = GetActive();
            if (!profile) {
                m_ActiveTransaction.clear();
                return;
            }

            auto& txs = profile->AllTransactions();

            // If current TX is invalid, pick first available
            if (txs.empty()) {
                m_ActiveTransaction.clear();
                return;
            }

            if (txs.find(m_ActiveTransaction) == txs.end()) {
                m_ActiveTransaction = txs.begin()->first;
            }
        }

        Profile& CreateProfile(const std::string& name,
                               const std::filesystem::path& dir) {
            auto path = dir / (name + ".json");

            auto profile = std::make_unique<Profile>(path);
            profile->SetName(name);
            profile->SaveToFile();  // initialize file

            auto [it, inserted] = m_Profiles.emplace(name, std::move(profile));

            if (!inserted) {
                CORE_ERROR("Profile {} already exists.", name);
            }

            return *it->second;
        }

        void SaveActive() {
            auto* p = GetActive();
            if (!p) {
                CORE_WARN("No active profile to save.");
                return;
            }
            p->SaveToFile();
        }

        void DeleteProfile(const std::string& name) {
            auto it = m_Profiles.find(name);
            if (it == m_Profiles.end())
                return;
            std::filesystem::remove(
                it->second->GetPath());  // you need this getter
            m_Profiles.erase(it);

            if (m_Active == name) {
                m_Active = m_Profiles.empty() ? "" : m_Profiles.begin()->first;
            }
        }
        void UnloadProfile(const std::string& name) {
            auto it = m_Profiles.find(name);
            if (it == m_Profiles.end())
                return;
            m_Profiles.erase(it);
        }
        void UnloadProfile(const std::filesystem::path& path) {
            for (const auto& [name, profile] : m_Profiles) {
                if (profile->GetPath() == path) {
                    UnloadProfile(name);
                    return;
                }
            }
        }
        bool RenameProfile(const std::string& oldName,
                           const std::string& newName) {
            auto it = m_Profiles.find(oldName);
            if (it == m_Profiles.end())
                return false;

            auto node = m_Profiles.extract(it);
            node.key() = newName;
            m_Profiles.insert(std::move(node));

            if (m_Active == oldName) {
                m_Active = newName;
            }

            return true;
        }

        std::optional<Profile> ExportProfile(const std::string& name) {
            auto it = m_Profiles.find(name);
            if (it == m_Profiles.end())
                return std::nullopt;

            return *it->second;  // copy
        }

        void ImportProfile(const Profile& profile) {
            auto name = profile.GetName();
            m_Profiles[name] = std::make_unique<Profile>(profile);
        }

        const auto& All() { return m_Profiles; }

      private:
        std::shared_ptr<NMTK::AppContext> m_AppContext;
        std::string m_Active;
        std::string m_ActiveTransaction;
        std::unordered_map<std::string, std::unique_ptr<Profile>> m_Profiles;
    };
}  // namespace OpenEFT
