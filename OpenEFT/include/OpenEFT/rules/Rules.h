#pragma once
#include <OpenEFT/rules/EnumHelpers.h>

#include <cstdint>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

namespace OpenEFT {

    // Presence semantics for fields, subfields, items, records
    DECLARE_ENUM(RULE_PRESENCE, uint8_t, UNKNOWN,
                 OPTIONAL,     // May appear
                 MANDATORY,    // Must appear
                 CONDITIONAL,  // Must appear if condition evaluates true
                 FORBIDDEN     // Must not appear (often legacy suppression)
    )

    // Atomic value representation
    DECLARE_ENUM(VALUE_TYPE, uint8_t, UNKNOWN, BOOL, INTEGER, UINT, FLOAT,
                 STRING, HEX_BINARY, B64_BINARY, BINARY, DATE, TIME, DATE_TIME,
                 ENUM, COMPOSITE)

    // Character encoding rules
    DECLARE_ENUM(CHAR_ENCODING, uint8_t, UNKNOWN, ASCII7, UTF8, BINARY)

    // Generic comparison operators
    DECLARE_ENUM(COMPARISON_OP, uint8_t, UNKNOWN, EQUAL, NOT_EQUAL, LESS,
                 LESS_EQUAL, GREATER, GREATER_EQUAL, IN_SET, NOT_IN_SET,
                 MATCH_REGEX)

    // Operators used when evaluating a condition
    DECLARE_ENUM(DEPENDENCY_OP, uint8_t, UNKNOWN, EXISTS, NOT_EXISTS, EQUALS,
                 NOT_EQUALS, IN_SET)

    // Rules governing groups of fields
    DECLARE_ENUM(EXCLUSIVITY_TYPE, uint8_t, UNKNOWN,
                 MUTUALLY_EXCLUSIVE,    // No more than one may exist
                 ONE_REQUIRED,          // At least one must exist
                 EXACTLY_ONE_REQUIRED,  // Exactly one must exist
                 AT_LEAST_ONE_REQUIRED  // One or more must exist
    )

    // Ordering constraints (traditional encoding)
    DECLARE_ENUM(ORDERING_RULE, uint8_t, UNKNOWN, MUST_BE_FIRST, MUST_BE_LAST,
                 FIXED_ORDER)

    // Allowed character classes for string fields
    DECLARE_ENUM(CHARACTER_CLASS, uint8_t, UNKNOWN, NUMERIC, ALPHA,
                 ALPHANUMERIC, UPPERCASE, HEX, PRINTABLE_ASCII)

    // Policy for unknown or unsupported fields
    DECLARE_ENUM(UNKNOWN_FIELD_POLICY, uint8_t, UNKNOWN, IGNORE, WARN, ERROR)

    // High-level semantic rule categories
    DECLARE_ENUM(SEMANTIC_RULE_TYPE, uint8_t, UNKNOWN,
                 FIELD_EQUALS_COMPUTED_VALUE, FIELD_MATCHES_COUNT,
                 FIELD_MATCHES_RECORD_COUNT, CUSTOM_VALIDATOR_HOOK)

    /* ============================================================
        Primitive Rule Components
       ============================================================ */

    DECLARE_ENUM(CONSTRAINT_TYPE, uint8_t, UNKNOWN, NUMERIC, ENUM, PATTERN)

    // Numeric or enumerated value constraint
    struct ValueConstraint {
        COMPARISON_OP op = COMPARISON_OP::EQUAL;
        CONSTRAINT_TYPE type = CONSTRAINT_TYPE::NUMERIC;
        // Numeric
        int64_t numeric_value = 0;
        // Enum reference
        std::string enum_name;   // e.g. "TOT"
        std::string enum_value;  // Slected value
        // Pattern
        std::string pattern;
        std::regex compiled_pattern;
    };

    // Identifies a specific field within a record
    struct FieldRef {
        uint16_t record_type;  // e.g. 10
        uint16_t field_id;     // e.g. 003
    };

    /* ============================================================
        Conditions (pure predicates, no enforcement)
       ============================================================ */

    // Single boolean test against a field
    struct FieldCondition {
        FieldRef field;
        DEPENDENCY_OP op;
        int64_t value;  // used for EQUALS / IN_SET
    };

    // Logical grouping of conditions
    DECLARE_ENUM(CONDITION_LOGIC, uint8_t, UNKNOWN, AND, OR)

    struct FieldConditionGroup {
        std::vector<FieldCondition> conditions;
        CONDITION_LOGIC logic;
    };

    /* ============================================================
        Enforcement Rules
       ============================================================ */

    // If <condition> then <action>
    struct FieldDependencyRule {
        FieldCondition if_condition;

        DEPENDENCY_OP then_op;  // EXISTS, NOT_EXISTS, EQUALS, etc.
        FieldRef then_field;
        int64_t then_value;  // used if then_op requires value
    };

    // Mutually exclusive or required field groups
    struct FieldGroupMember {
        uint16_t record_type;
        uint16_t field_id;
    };

    struct FieldGroupRule {
        EXCLUSIVITY_TYPE type;
        std::vector<FieldGroupMember> members;
    };

    // Cross-record referential integrity
    struct LinkRule {
        FieldRef source;
        FieldRef target;
        bool must_exist;
    };

    // Conditional enforcement of a value constraint
    struct ConditionalConstraint {
        FieldCondition trigger;
        FieldRef target;
        ValueConstraint enforced;
    };

    // Record composition rules at transaction level
    struct RecordSetRule {
        std::vector<uint16_t> record_types;
        uint32_t min_total;
        uint32_t max_total;
    };

    /* ============================================================
        Structural Rules (bottom-up)
       ============================================================ */

    // Smallest atomic value
    struct ItemRule {
        std::string item_name;
        std::string item_short;
        VALUE_TYPE type;
        RULE_PRESENCE presence;

        uint32_t min_length;
        uint32_t max_length;

        std::vector<ValueConstraint> constraints;
    };

    // Repeatable composite element inside a field
    struct SubfieldRule {
        std::string subfield_name;
        std::string subfield_short;
        uint16_t subfield_id;  // local identifier
        RULE_PRESENCE presence;

        uint32_t min_occurs;
        uint32_t max_occurs;

        std::vector<ItemRule> items;
    };

    // ANSI/NIST / EBTS field definition
    struct FieldRule {
        std::string field_name;
        std::string field_short;
        uint16_t record_type;
        uint16_t field_id;

        RULE_PRESENCE presence;

        // Optional condition controlling CONDITIONAL / FORBIDDEN
        std::vector<FieldConditionGroup> presence_conditions;

        VALUE_TYPE type;
        CHAR_ENCODING encoding;

        uint32_t min_length = 0;
        uint32_t max_length = 0;

        uint32_t min_occurs = 0;
        uint32_t max_occurs = 0;

        std::vector<SubfieldRule> subfields;
        std::vector<ValueConstraint> constraints;
    };

    // Record-level structure and dependencies
    struct RecordRule {
        std::string record_name;
        std::string record_short;
        uint16_t record_type;

        uint32_t min_occurs;
        uint32_t max_occurs;

        // Optional condition gating applicability of this record
        std::vector<FieldConditionGroup> applies_when;

        std::vector<FieldRule> fields;
        std::vector<FieldDependencyRule> dependencies;
    };

    /* ============================================================
        Semantic & Transaction-Level Rules
       ============================================================ */

    struct SemanticRule {
        SEMANTIC_RULE_TYPE type;
        FieldRef target;
        uint32_t parameter;  // meaning defined by rule type
    };

    struct TransactionRule {
        std::string name;
        std::string transaction_type;  // e.g. CAR, FAUF mapped to ID
        std::string domain_id;         // EBTS DOM / profile

        UNKNOWN_FIELD_POLICY unknown_field_policy;

        std::vector<RecordRule> records;
        std::vector<RecordSetRule> record_sets;
        std::vector<FieldGroupRule> field_groups;
        std::vector<LinkRule> link_rules;
        std::vector<ConditionalConstraint> conditional_constraints;
        std::vector<SemanticRule> semantic_rules;
    };

}  // namespace OpenEFT