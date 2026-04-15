#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <nnist/nnist.h>

namespace OpenEFT {
    enum class SEVERITY : uint8_t {
        INFO,
        WARNING,
        ERROR
    };

    enum class RULE_VIOLATION : uint16_t {
        // Record Sets
        RECORD_SET_TOO_FEW,
        RECORD_SET_TOO_MANY,
        // Records
        MISSING_MANDATORY_RECORD,
        RECORD_COUNT_OUT_OF_RANGE,
        UNKNOWN_RECORD_PRESENT,
        // Fields
        MISSING_MANDATORY_FIELD,
        FORBIDDEN_FIELD_PRESENT,
        FIELD_OCCURS_OUT_OF_RANGE,
        FIELD_LENGTH_TOO_SHORT,
        FIELD_LENGTH_TOO_LONG,
        UNSUPPORTED_FIELD_PRESENT,
        // Subfields
        MISSING_MANDATORY_SUBFIELD,
        SUBFIELD_LENGTH_TOO_LONG,
        SUBFIELD_LENGTH_TOO_SHORT,
        SUBFIELD_OCCURS_TOO_FEW,
        SUBFIELD_OCCURS_TOO_MANY,
        SUBFIELD_TOO_FEW_ITEMS,
        SUBFIELD_TOO_MANY_ITEMS,
        // Items
        INVALID_TYPE,
        ITEM_LENGTH_TOO_LONG,
        ITEM_LENGTH_TOO_SHORT,
        MISSING_MANDATORY_ITEM,
        // Enums
        VALUE_NOT_IN_ENUM,
        VALUE_BELOW_MIN,
        VALUE_ABOVE_MAX,
        PATTERN_MISMATCH,
    };

    struct Diagnostic {
        SEVERITY severity;
        RULE_VIOLATION code;

        uint16_t record_type = 0;
        uint16_t field_id = 0;

        // Optional: which occurrence index, etc.
        uint32_t observed = 0;
        uint32_t expected_min = 0;
        uint32_t expected_max = 0;

        std::string message;  // keep it simple for now
    };

    struct FieldInstanceView {
        // For v1, represent the raw field payload (already extracted)
        // Could be the full field text or a specific item text.
        const nnist::Record* record;
        const nnist::Field* field;
        uint32_t record_index;
        uint32_t field_index;
    };
}  // namespace OpenEFT
