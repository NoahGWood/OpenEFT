// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#include <OpenEFT/profile/ProfileManager.h>
namespace OpenEFT {
    void ProfileManager::LoadProfiles(const std::filesystem::path& dir) {
        m_Profiles.clear();

        if (!std::filesystem::exists(dir)) {
            CORE_WARN("Profile directory does not exist: {}", dir.string());
            return;
        }

        for (auto& entry : std::filesystem::directory_iterator(dir)) {
            if (!entry.is_regular_file())
                continue;
            if (entry.path().extension() != ".json")
                continue;

            try {
                auto profile = std::make_unique<Profile>(entry.path());

                std::string name = entry.path().stem().string();
                m_Profiles.emplace(name, std::move(profile));

            } catch (...) {
                CORE_ERROR("Failed to load profile: {}", entry.path().string());
            }
        }

        if (m_Active.empty() && !m_Profiles.empty()) {
            m_Active = m_Profiles.begin()->first;
        }
    }
    void ProfileManager::LoadProfile(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            CORE_WARN("Profile does not exist: {}", path.string());
            return;
        }
        if (path.extension() != ".json") {
            CORE_WARN("{} is not a profile.", path.string());
            return;
        }

        try {
            auto profile = std::make_unique<Profile>(path);
            std::string name = path.stem().string();
            m_Profiles.emplace(name, std::move(profile));
            m_Active = name;
        } catch (...) {
            CORE_ERROR("Failed to load profile: {}", path.string());
        }
    }
    void ProfileManager::SaveAll() {
        for (auto& [name, profile] : m_Profiles) {
            profile->SaveToFile();
        }
    }
}  // namespace OpenEFT
