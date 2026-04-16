// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <OpenEFT/rules/RuleMacros.h>
#include <OpenEFT/rules/Rules.h>

#include <nlohmann/json.hpp>

namespace OpenEFT {
    using json = nlohmann::json;

    // ============================================================
    // Helpers
    // ============================================================

    template <typename T>
    inline void get_if_exists(const json& j, const char* key, T& out) {
        if (j.contains(key)) {
            out = j.at(key).get<T>();
        }
    }

#define JSON_TO_FIELD(j, obj, field) j[#field] = obj.field;
#define JSON_FROM_FIELD(j, obj, field) \
    if (j.contains(#field))            \
        j.at(#field).get_to(obj.field);

#define JSON_TO_ENUM(j, obj, field) j[#field] = ToString(obj.field);
#define JSON_FROM_ENUM(j, obj, field, ENUM) \
    obj.field = FromString_##ENUM(j.value(#field, "UNKNOWN"));

    // ============================================================
    // ValueConstraint
    // ============================================================

    inline void to_json(json& j, const ValueConstraint& v) {
#define X(field) JSON_TO_FIELD(j, v, field)
        VALUE_CONSTRAINT_FIELDS(X)
#undef X
        j["type"] = ToString(v.type);
        j["op"] = ToString(v.op);  // override enum
    }

    inline void from_json(const json& j, ValueConstraint& v) {
#define X(field) JSON_FROM_FIELD(j, v, field)
        VALUE_CONSTRAINT_FIELDS(X)
#undef X
        v.op = FromString_COMPARISON_OP(j.value("op", "UNKNOWN"));

        if (!v.pattern.empty()) {
            v.compiled_pattern = std::regex(v.pattern);
        }
    }
    // ============================================================
    // FieldRef
    // ============================================================

    inline void to_json(json& j, const FieldRef& f) {
#define X(field) JSON_TO_FIELD(j, f, field)
        FIELD_REF_FIELDS(X)
#undef X
    }

    inline void from_json(const json& j, FieldRef& f) {
#define X(field) JSON_FROM_FIELD(j, f, field)
        FIELD_REF_FIELDS(X)
#undef X
    }
    // ============================================================
    // FieldCondition
    // ============================================================

    inline void to_json(json& j, const FieldCondition& c) {
#define X(field) JSON_TO_FIELD(j, c, field)
        FIELD_CONDITION_FIELDS(X)
#undef X

        j["op"] = ToString(c.op);
    }

    inline void from_json(const json& j, FieldCondition& c) {
#define X(field) JSON_FROM_FIELD(j, c, field)
        FIELD_CONDITION_FIELDS(X)
#undef X

        c.op = FromString_DEPENDENCY_OP(j.value("op", "UNKNOWN"));
    }

    // ============================================================
    // FieldConditionGroup
    // ============================================================

    inline void to_json(json& j, const FieldConditionGroup& g) {
        j["conditions"] = g.conditions;
        j["logic"] = ToString(g.logic);
    }

    inline void from_json(const json& j, FieldConditionGroup& g) {
        g.conditions = j.value("conditions", std::vector<FieldCondition>{});
        g.logic = FromString_CONDITION_LOGIC(j.value("logic", "UNKNOWN"));
    }

    // ============================================================
    // FieldDependencyRule
    // ============================================================

    inline void to_json(json& j, const FieldDependencyRule& r) {
#define X(field) JSON_TO_FIELD(j, r, field)
        FIELD_DEPENDENCY_RULE_FIELDS(X)
#undef X

        j["then_op"] = ToString(r.then_op);
    }

    inline void from_json(const json& j, FieldDependencyRule& r) {
#define X(field) JSON_FROM_FIELD(j, r, field)
        FIELD_DEPENDENCY_RULE_FIELDS(X)
#undef X

        r.then_op = FromString_DEPENDENCY_OP(j.value("then_op", "UNKNOWN"));
    }

    // ============================================================
    // FieldGroupMember
    // ============================================================

    inline void to_json(json& j, const FieldGroupMember& m) {
#define X(field) JSON_TO_FIELD(j, m, field)
        FIELD_GROUP_MEMBER_FIELDS(X)
#undef X
    }

    inline void from_json(const json& j, FieldGroupMember& m) {
#define X(field) JSON_FROM_FIELD(j, m, field)
        FIELD_GROUP_MEMBER_FIELDS(X)
#undef X
    }

    // ============================================================
    // FieldGroupRule
    // ============================================================

    inline void to_json(json& j, const FieldGroupRule& r) {
        j["type"] = ToString(r.type);
        j["members"] = r.members;
    }

    inline void from_json(const json& j, FieldGroupRule& r) {
        r.type = FromString_EXCLUSIVITY_TYPE(j.value("type", "UNKNOWN"));
        r.members = j.value("members", std::vector<FieldGroupMember>{});
    }

    // ============================================================
    // LinkRule
    // ============================================================

    inline void to_json(json& j, const LinkRule& r) {
#define X(field) JSON_TO_FIELD(j, r, field)
        LINK_RULE_FIELDS(X)
#undef X
    }

    inline void from_json(const json& j, LinkRule& r) {
#define X(field) JSON_FROM_FIELD(j, r, field)
        LINK_RULE_FIELDS(X)
#undef X
    }

    // ============================================================
    // ConditionalConstraint
    // ============================================================
    inline void to_json(json& j, const ConditionalConstraint& c) {
#define X(field) JSON_TO_FIELD(j, c, field)
        CONDITIONAL_CONSTRAINT_FIELDS(X)
#undef X
    }

    inline void from_json(const json& j, ConditionalConstraint& c) {
#define X(field) JSON_FROM_FIELD(j, c, field)
        CONDITIONAL_CONSTRAINT_FIELDS(X)
#undef X
    }
    // ============================================================
    // RecordSetRule
    // ============================================================

    inline void to_json(json& j, const RecordSetRule& r) {
        j = {{"record_types", r.record_types},
             {"min_total", r.min_total},
             {"max_total", r.max_total}};
    }

    inline void from_json(const json& j, RecordSetRule& r) {
        r.record_types = j.value("record_types", std::vector<uint16_t>{});
        r.min_total = j.value("min_total", 0);
        r.max_total = j.value("max_total", 0);
    }

    // ============================================================
    // ItemRule
    // ============================================================

    inline void to_json(json& j, const ItemRule& i) {
#define X(field) JSON_TO_FIELD(j, i, field)
        ITEM_RULE_FIELDS(X)
#undef X
        j["item_name"] = i.item_name;
        j["item_short"] = i.item_short;
        j["type"] = ToString(i.type);
        j["presence"] = ToString(i.presence);
    }

    inline void from_json(const json& j, ItemRule& i) {
#define X(field) JSON_FROM_FIELD(j, i, field)
        ITEM_RULE_FIELDS(X)
#undef X
        i.item_name = j.value("item_name", "");
        i.item_short = j.value("item_short", "");
        i.type = FromString_VALUE_TYPE(j.value("type", "UNKNOWN"));
        i.presence = FromString_RULE_PRESENCE(j.value("presence", "UNKNOWN"));
    }
    // ============================================================
    // SubfieldRule
    // ============================================================

    inline void to_json(json& j, const SubfieldRule& s) {
#define X(field) JSON_TO_FIELD(j, s, field)
        SUBFIELD_RULE_FIELDS(X)
#undef X
        j["subfield_name"] = s.subfield_name;
        j["subfield_short"] = s.subfield_short;
        j["presence"] = ToString(s.presence);
    }

    inline void from_json(const json& j, SubfieldRule& s) {
#define X(field) JSON_FROM_FIELD(j, s, field)
        SUBFIELD_RULE_FIELDS(X)
#undef X
        s.subfield_name = j.value("subfield_name", "");
        s.subfield_short = j.value("subfield_short", "");
        s.presence = FromString_RULE_PRESENCE(j.value("presence", "UNKNOWN"));
    }

    // ============================================================
    // FieldRule
    // ============================================================

    inline void to_json(json& j, const FieldRule& f) {
#define X(field) JSON_TO_FIELD(j, f, field)
        FIELD_RULE_FIELDS(X)
#undef X
        j["field_name"] = f.field_name;
        j["field_short"] = f.field_short;
        j["presence"] = ToString(f.presence);
        j["type"] = ToString(f.type);
        j["encoding"] = ToString(f.encoding);
    }

    inline void from_json(const json& j, FieldRule& f) {
#define X(field) JSON_FROM_FIELD(j, f, field)
        FIELD_RULE_FIELDS(X)
#undef X

        f.field_name = j.value("field_name", "");
        f.field_short = j.value("field_short", "");
        f.presence = FromString_RULE_PRESENCE(j.value("presence", "UNKNOWN"));
        f.type = FromString_VALUE_TYPE(j.value("type", "UNKNOWN"));
        f.encoding = FromString_CHAR_ENCODING(j.value("encoding", "UNKNOWN"));
    }
    // ============================================================
    // RecordRule
    // ============================================================
    inline void to_json(json& j, const RecordRule& r) {
#define X(field) JSON_TO_FIELD(j, r, field)
        RECORD_RULE_FIELDS(X)
#undef X
        j["record_name"] = r.record_name;
        j["record_short"] = r.record_short;
    }

    inline void from_json(const json& j, RecordRule& r) {
#define X(field) JSON_FROM_FIELD(j, r, field)
        RECORD_RULE_FIELDS(X)
#undef X
        r.record_name = j.value("record_name", "");
        r.record_short = j.value("record_short", "");
    }
    // ============================================================
    // SemanticRule
    // ============================================================

    inline void to_json(json& j, const SemanticRule& s) {
#define X(field) JSON_TO_FIELD(j, s, field)
        SEMANTIC_RULE_FIELDS(X)
#undef X

        j["type"] = ToString(s.type);
    }

    inline void from_json(const json& j, SemanticRule& s) {
#define X(field) JSON_FROM_FIELD(j, s, field)
        SEMANTIC_RULE_FIELDS(X)
#undef X

        s.type = FromString_SEMANTIC_RULE_TYPE(j.value("type", "UNKNOWN"));
    }

    // ============================================================
    // TransactionRule
    // ============================================================

    inline void to_json(json& j, const TransactionRule& t) {
#define X(field) JSON_TO_FIELD(j, t, field)
        TRANSACTION_RULE_FIELDS(X)
#undef X
        j["unknown_field_policy"] = ToString(t.unknown_field_policy);
    }

    inline void from_json(const json& j, TransactionRule& t) {
#define X(field) JSON_FROM_FIELD(j, t, field)
        TRANSACTION_RULE_FIELDS(X)
#undef X
        t.unknown_field_policy = FromString_UNKNOWN_FIELD_POLICY(
            j.value("unknown_field_policy", "UNKNOWN"));
    }
}  // namespace OpenEFT
