// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#include <OpenEFT/profile/Profile.h>

#include <fstream>
#include <nlohmann/json.hpp>

namespace OpenEFT {
    void Profile::SaveToFile() {
        json j = *this;

        std::ofstream out(m_File);
        if(!out){
            CORE_ERROR("Failed to open file for writing: {}", m_File.string());
            return;
        }
        out << j.dump(4);
    }
    void Profile::LoadFromFile() {
        if(!std::filesystem::exists(m_File)){
            CORE_WARN("Profile file does not exist: {}", m_File.string());
            return;
        }
        std::ifstream in(m_File);
        if(!in.is_open()){
            CORE_ERROR("Failed to open file for reading: {}", m_File.string());
            return;
        }

        json j;
        in >> j;
        Profile temp;
        from_json(j, temp);
        temp.m_File = m_File; // Preserve path
        // Migration hook
        if(temp.m_Version < 1) {
            CORE_WARN("Migrating profile from version {}", temp.m_Version);
            temp.m_Version = 1;
        }
        *this = std::move(temp);
    }
}  // namespace OpenEFT
