// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <NMTK/NMTK.h>
#include <OpenEFT/validator/Data.h>

#include <algorithm>
#include <string>
#include <vector>

namespace OpenEFT {
    inline static uint16_t ParseFieldId(const std::vector<uint8_t>& tag) {
        // Expect something like "2.001"
        // We only care about numeric portion after '.'
        auto dot = std::find(tag.begin(), tag.end(), '.');
        if (dot == tag.end())
            return 0;

        std::string num(dot + 1, tag.end());
        return static_cast<uint16_t>(std::stoi(num));
    }

    inline static const char* SeverityText(SEVERITY s) {
        switch (s) {
            case SEVERITY::ERROR:
                return "Error";
            case SEVERITY::WARNING:
                return "Warning";
            case SEVERITY::INFO:
                return "Info";
            default:
                return "Unknown";
        }
    }
    inline static ImVec4 SeverityColor(SEVERITY s) {
        switch (s) {
            case SEVERITY::ERROR:
                return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
            case SEVERITY::WARNING:
                return ImVec4(1.0f, 0.8f, 0.3f, 1.0f);
            case SEVERITY::INFO:
                return ImVec4(0.4f, 0.8f, 1.0f, 1.0f);
            default:
                return ImVec4(1, 1, 1, 1);
        }
    }
}  // namespace OpenEFT
