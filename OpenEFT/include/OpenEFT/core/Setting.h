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
