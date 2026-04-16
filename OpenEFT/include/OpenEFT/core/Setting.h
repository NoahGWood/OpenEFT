// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <filesystem>
namespace OpenEFT
{
    struct Setting {
        // Validator
        bool strict_enums = true;
        bool strict_occurrence = true;
        bool allow_unknown_fields = true;
        bool auto_validate = true;
        // UI
        bool show_hex_view = false;
        bool highlight_errors = true;
    }; 
} // namespace OpenEFT
