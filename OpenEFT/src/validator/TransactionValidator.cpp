// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#include <OpenEFT/validator/Helpers.h>
#include <OpenEFT/validator/TransactionValidator.h>
#include <unordered_set>

namespace OpenEFT {
    std::vector<Diagnostic> TransactionValidator::Validate(
        const TransactionRule& tx, const DocumentView& doc) const {
        std::vector<Diagnostic> diags;
        std::unordered_map<uint16_t, std::unordered_set<uint16_t>> allowed;
        for (uint32_t i = 0; i < tx.records.size(); ++i) {
            const RecordRule& rr = tx.records[i];
            for (uint32_t j = 0; j < rr.fields.size(); ++j) {
                const FieldRule& fr = rr.fields[j];
                allowed[rr.record_type].insert(fr.field_id);
            }
        }
        auto records = doc.Records();
        // ---- Iterate over actual records ----
        for (const auto& record : records) {
            auto it = allowed.find(record.type);
            // ---- Identify unknown records ----
            if (m_Opt.validate_unknown_records) {
                if (it == allowed.end()) {
                    UnknownRecord(tx, record.type, diags);
                    continue;
                }
            }
            // ---- Identify unknown fields ----
            if (m_Opt.validate_unknown_fields) {
                const auto& allowedFields = it->second;
                for (const auto& field : record.fields) {
                    uint16_t fid = ParseFieldId(field.tag);
                    if (!allowedFields.contains(fid)) {
                        UnknownField(tx, record.type, fid, diags);
                    }
                }
            }
        }

        // 1) Records
        for (const auto& rr : tx.records) {
            ValidateRecord(rr, doc, diags);
        }

        // 2) Record sets, groups, links, etc later…
        ValidateRecordSets(tx, doc, diags);

        return diags;
    }

    SEVERITY TransactionValidator::PolicyToSeverity(
        UNKNOWN_FIELD_POLICY p) {
        switch (p) {
            case UNKNOWN_FIELD_POLICY::IGNORE:
                return SEVERITY::INFO;
            case UNKNOWN_FIELD_POLICY::WARN:
                return SEVERITY::WARNING;
            case UNKNOWN_FIELD_POLICY::ERROR:
                return SEVERITY::ERROR;
            default:
                break;
        }
        return SEVERITY::WARNING;
    }

    void TransactionValidator::ValidateRecordSets(
        const TransactionRule& tx, const DocumentView& doc,
        std::vector<Diagnostic>& out) const {
        //
        for (const auto& rs : tx.record_sets) {
            uint32_t total = 0;
            for(uint16_t rt : rs.record_types){
                total += doc.RecordCount(rt);
            }
            if (rs.min_total && total < rs.min_total) {
                out.push_back(MakeRecordSetDiag(
                    SEVERITY::ERROR, RULE_VIOLATION::RECORD_SET_TOO_FEW, total,
                    rs.min_total, rs.max_total));
            }
            if (rs.max_total && rs.max_total != UINT32_MAX &&
                total > rs.max_total) {
                out.push_back(MakeRecordSetDiag(
                    SEVERITY::ERROR, RULE_VIOLATION::RECORD_SET_TOO_MANY, total,
                    rs.min_total, rs.max_total));
            }
        }
    }

    void TransactionValidator::ValidateRecord(
        const RecordRule& rr, const DocumentView& doc,
        std::vector<Diagnostic>& out) const {
        const uint32_t recCount = doc.RecordCount(rr.record_type);

        // Record count range
        if (rr.min_occurs && recCount < rr.min_occurs) {
            out.push_back(MakeRecordDiag(
                SEVERITY::ERROR, RULE_VIOLATION::MISSING_MANDATORY_RECORD,
                rr.record_type, recCount, rr.min_occurs, rr.max_occurs,
                "Missing mandatory record"));
        }
        if (rr.max_occurs && rr.max_occurs != UINT32_MAX &&
            recCount > rr.max_occurs) {
            out.push_back(MakeRecordDiag(
                SEVERITY::ERROR, RULE_VIOLATION::RECORD_COUNT_OUT_OF_RANGE,
                rr.record_type, recCount, rr.min_occurs, rr.max_occurs,
                "Record count out of range"));
        }

        // Fields
        for(const auto& fr : rr.fields){
            ValidateField(fr, doc, out);

        }
    }

    void TransactionValidator::ValidateField(
        const FieldRule& fr, const DocumentView& doc,
        std::vector<Diagnostic>& out) const {
        const uint32_t count = doc.FieldCount(fr.record_type, fr.field_id);

        //---- Aggregation checks ----
        // Validate the presence of the field if rule requires.
        if (fr.presence == RULE_PRESENCE::MANDATORY && count == 0) {
            out.push_back(MakeFieldDiag(SEVERITY::ERROR,
                                        RULE_VIOLATION::MISSING_MANDATORY_FIELD,
                                        fr, count, fr.min_occurs, fr.max_occurs,
                                        "Missing mandatory field"));
            return;
        }
        if (fr.presence == RULE_PRESENCE::FORBIDDEN && count > 0) {
            out.push_back(MakeFieldDiag(
                SEVERITY::ERROR, RULE_VIOLATION::FORBIDDEN_FIELD_PRESENT, fr,
                count, 0, 0, "Forbidden field present"));
        }
        // Validate the min/max occurs of the fields
        // Occurs range (only if present)
        if (m_Opt.validate_occurs && count > 0) {
            if (count < fr.min_occurs) {
                out.push_back(MakeFieldDiag(
                    SEVERITY::ERROR, RULE_VIOLATION::FIELD_OCCURS_OUT_OF_RANGE,
                    fr, count, fr.min_occurs, fr.max_occurs,
                    "Field occurrences below minOccurs"));
            }
            const uint32_t maxOccurs =
                fr.max_occurs == 0 ? UINT32_MAX : fr.max_occurs;
            if (maxOccurs != UINT32_MAX && count > maxOccurs) {
                out.push_back(MakeFieldDiag(
                    SEVERITY::ERROR, RULE_VIOLATION::FIELD_OCCURS_OUT_OF_RANGE,
                    fr, count, fr.min_occurs, fr.max_occurs,
                    "Field occurrences exceed maxOccurs"));
            }
        }

        //---- Instance checks ----
        doc.ForEachFieldInstance(fr.record_type, fr.field_id,
                                 [&](const FieldInstanceView& v) {
                                     ValidateFieldInstance(fr, v, out);
                                 });
        // Validate subfields
        // if (fr.subfield_count > 0) {
            // for (uint32_t sfi = 0; sfi < fr.subfield_count; ++sfi) {
            //     const SubfieldRule& sf = fr.subfields[sfi];
            //     ValidateSubfield(sf, doc, out);
            // }
        // }
    }

    void TransactionValidator::ValidateFieldInstance(
        const FieldRule& fr, const FieldInstanceView& view,
        std::vector<Diagnostic>& out) const {
        // Start with raw field size
        size_t len = view.field->raw_field.size();
        // compute sf size if exists
        if (view.field->subfields.size() > 0) {
            for (const auto& sf : view.field->subfields) {
                for (const auto& itm : sf.items) {
                    len += itm.bytes.size();
                }
            }
        }
        // Handle binary cases
        // if(fr.type == VALUE_TYPE::BINARY){
        //     len = len / 2;
        // }
        // Handle length facets
        if (fr.min_length && len < fr.min_length) {
            out.push_back(MakeFieldDiag(SEVERITY::ERROR,
                                        RULE_VIOLATION::FIELD_LENGTH_TOO_SHORT,
                                        fr, len, fr.min_length, fr.max_length,
                                        "Field length below minLength"));
        }
        if (fr.max_length && fr.max_length != UINT32_MAX &&
            len > fr.max_length) {
            out.push_back(MakeFieldDiag(SEVERITY::ERROR,
                                        RULE_VIOLATION::FIELD_LENGTH_TOO_LONG,
                                        fr, len, fr.min_length, fr.max_length,
                                        "Field length exceeds maxLength"));
        }
        // Validate subfields
        for(const auto& sfr : fr.subfields){
            ValidateSubfield(sfr, *view.field, out);
        }
    }
    void TransactionValidator::ValidateSubfield(
        const SubfieldRule& sfr, const nnist::Field& field,
        std::vector<Diagnostic>& out) const {
        uint32_t count = field.subfields.size();
        // Validate the presence of subfield if required
        if (sfr.presence == RULE_PRESENCE::MANDATORY && count == 0) {
            out.push_back(MakeSubfieldDiag(
                SEVERITY::ERROR, RULE_VIOLATION::MISSING_MANDATORY_SUBFIELD,
                sfr, 0, sfr.min_occurs, sfr.max_occurs,
                "Missing required subfield."));
        }
        // Validate subfield occurs min/max times
        if (count < sfr.min_occurs) {
            out.push_back(MakeSubfieldDiag(
                SEVERITY::ERROR, RULE_VIOLATION::SUBFIELD_OCCURS_TOO_FEW, sfr,
                count, sfr.min_occurs, sfr.max_occurs,
                "Subfield occurs too few times."));
        }
        if (count > sfr.max_occurs) {
            out.push_back(MakeSubfieldDiag(
                SEVERITY::ERROR, RULE_VIOLATION::SUBFIELD_OCCURS_TOO_MANY, sfr,
                count, sfr.min_occurs, sfr.max_occurs,
                "Subfield occurs too many times."));
        }
        // Recurse to item rules
        for (uint32_t i = 0; i < count; ++i) {
            const auto& sub = field.subfields[i];
            
            for (uint32_t j = 0; j < sfr.items.size(); ++j) {
                const auto& ir = sfr.items[j];
                if (j < sub.items.size()) {
                    ValidateItem(ir, sub.items[j], out);
                } else if (ir.presence == RULE_PRESENCE::MANDATORY) {
                    out.push_back(MakeItemDiag(
                        SEVERITY::ERROR, RULE_VIOLATION::MISSING_MANDATORY_ITEM,
                        ir, count, sfr.min_occurs, sfr.max_occurs,
                        "Mandatory item missing."));
                }
            }
        }
    }
    void TransactionValidator::ValidateItem(
        const ItemRule& rule, const nnist::Item& item,
        std::vector<Diagnostic>& out) const {
        // Validate the data type matches the actual value recorded.
        if (!ValidateItemType(rule, item, out)) {
            return;  // Can't test anyway, so skipit
        }
        // Validate the item min/max length
        uint32_t len = item.bytes.size();
        if (rule.min_length && len < rule.min_length) {
            out.push_back(MakeItemDiag(
                SEVERITY::ERROR, RULE_VIOLATION::ITEM_LENGTH_TOO_SHORT, rule,
                len, rule.min_length, rule.max_length,
                "Item length too short"));
        }
        if (rule.max_length && len > rule.max_length) {
            out.push_back(MakeItemDiag(
                SEVERITY::ERROR, RULE_VIOLATION::ITEM_LENGTH_TOO_LONG, rule,
                len, rule.min_length, rule.max_length, "Item length too long"));
        }
        // Validate constraints (numerical min/max, enumeration, etc.)
        ValidateItemConstraints(rule, item, out);
    }

    // Helper
    static std::string AsText(const nnist::Item& item) {
        return std::string(reinterpret_cast<const char*>(item.bytes.data()),
                           item.bytes.size());
    }

    bool TransactionValidator::ValidateItemType(
        const ItemRule& rule, const nnist::Item& view,
        std::vector<Diagnostic>& out) const {

        auto s = AsText(view);

        auto error = [&]() {
            out.push_back(MakeItemDiag(SEVERITY::ERROR,
                                       RULE_VIOLATION::INVALID_TYPE, rule, 1, 0,
                                       0, s));
        };

        if (s.empty()) {
            error();
            return false;
        }

        bool valid = true;

        switch (rule.type) {
            case VALUE_TYPE::BOOL: {
                if (!(s == "0" || s == "1"))
                    valid = false;
                break;
            }

            case VALUE_TYPE::INTEGER: {
                size_t i = 0;
                if (s[0] == '-')
                    i = 1;

                if (i == s.size())
                    valid = false;

                for (; i < s.size() && valid; ++i) {
                    if (!std::isdigit(static_cast<unsigned char>(s[i])))
                        valid = false;
                }

                break;
            }

            case VALUE_TYPE::UINT: {
                for (size_t i = 0; i < s.size() && valid; ++i) {
                    if (!std::isdigit(static_cast<unsigned char>(s[i])))
                        valid = false;
                }
                break;
            }

            case VALUE_TYPE::FLOAT: {
                size_t i = 0;
                if (s[0] == '-')
                    i = 1;

                if (i == s.size()) {
                    valid = false;
                    break;
                }

                bool seen_dot = false;
                bool seen_digit = false;

                for (; i < s.size() && valid; ++i) {
                    char c = s[i];

                    if (std::isdigit(static_cast<unsigned char>(c))) {
                        seen_digit = true;
                    } else if (c == '.') {
                        if (seen_dot)
                            valid = false;
                        seen_dot = true;
                    } else {
                        valid = false;
                    }
                }

                if (!seen_digit)
                    valid = false;

                break;
            }

            case VALUE_TYPE::DATE:  // YYYYMMDD
            {
                if (s.size() != 8) {
                    valid = false;
                    break;
                }

                for (char c : s) {
                    if (!std::isdigit(static_cast<unsigned char>(c))) {
                        valid = false;
                        break;
                    }
                }
                break;
            }

            case VALUE_TYPE::TIME:  // HHMMSS
            {
                if (s.size() != 6) {
                    valid = false;
                    break;
                }

                for (char c : s) {
                    if (!std::isdigit(static_cast<unsigned char>(c))) {
                        valid = false;
                        break;
                    }
                }
                break;
            }

            case VALUE_TYPE::DATE_TIME:  // YYYYMMDDHHMMSS
            {
                if (s.size() != 14) {
                    valid = false;
                    break;
                }

                for (char c : s) {
                    if (!std::isdigit(static_cast<unsigned char>(c))) {
                        valid = false;
                        break;
                    }
                }
                break;
            }
            default:
                break;
            // case VALUE_TYPE::HEX_BINARY:
            // case VALUE_TYPE::B64_BINARY:
            // case VALUE_TYPE::STRING:
            // case VALUE_TYPE::BINARY:
            // case VALUE_TYPE::ENUM:
            // case VALUE_TYPE::COMPOSITE:
                break;
        }

        if (!valid)
            error();
        return valid;
    }

    void TransactionValidator::ValidateItemConstraints(
        const ItemRule& rule, const nnist::Item& view,
        std::vector<Diagnostic>& out) const {
        if (rule.constraints.size() == 0) {
            return;
        }
        std::string s = AsText(view);

        for (uint32_t i = 0; i < rule.constraints.size(); ++i) {
            const auto& c = rule.constraints[i];

            switch (c.op) {
                case COMPARISON_OP::IN_SET: {
                    bool found = false;
                    // for (const auto& allowed : c.allowed_values) {
                    //     if (s == allowed) {
                    //         found = true;
                    //         break;
                    //     }
                    // }
                    if (!found) {
                        out.push_back(MakeItemDiag(
                            SEVERITY::ERROR, RULE_VIOLATION::VALUE_NOT_IN_ENUM,
                            rule, 1, 0, 0, "Value not in allowed set"));
                    }
                    break;
                }

                case COMPARISON_OP::GREATER_EQUAL: {
                    if (rule.type == VALUE_TYPE::INTEGER ||
                        rule.type == VALUE_TYPE::UINT ||
                        rule.type == VALUE_TYPE::FLOAT) {
                        double val = std::stod(s);
                        if (val < c.numeric_value) {
                            out.push_back(MakeItemDiag(
                                SEVERITY::ERROR,
                                RULE_VIOLATION::VALUE_BELOW_MIN, rule, 1, 0, 0,
                                "Value below minimum"));
                        }
                    }
                    break;
                }

                case COMPARISON_OP::LESS_EQUAL: {
                    if (rule.type == VALUE_TYPE::INTEGER ||
                        rule.type == VALUE_TYPE::UINT ||
                        rule.type == VALUE_TYPE::FLOAT) {
                        double val = std::stod(s);
                        if (val > c.numeric_value) {
                            out.push_back(MakeItemDiag(
                                SEVERITY::ERROR,
                                RULE_VIOLATION::VALUE_ABOVE_MAX, rule, 1, 0, 0,
                                "Value above maximum"));
                        }
                    }
                    break;
                }

                case COMPARISON_OP::MATCH_REGEX: {
                    if (!std::regex_match(s, std::regex(c.pattern))) {
                        out.push_back(MakeItemDiag(
                            SEVERITY::ERROR, RULE_VIOLATION::PATTERN_MISMATCH,
                            rule, 1, 0, 0,
                            "Value does not match required pattern"));
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }

    Diagnostic TransactionValidator::MakeRecordSetDiag(SEVERITY sev,
                                                       RULE_VIOLATION code,
                                                       uint32_t total,
                                                       uint32_t min,
                                                       uint32_t max) {
        Diagnostic d{};
        d.severity = sev;
        d.code = code;
        d.observed = total;
        d.expected_min = min;
        d.expected_max = max;
        return d;
    }

    Diagnostic TransactionValidator::MakeRecordDiag(
        SEVERITY sev, RULE_VIOLATION code, uint16_t rt, uint32_t observed,
        uint32_t min, uint32_t max, std::string msg) {
        Diagnostic d{};
        d.severity = sev;
        d.code = code;
        d.record_type = rt;
        d.observed = observed;
        d.expected_min = min;
        d.expected_max = max;
        d.message = std::move(msg);
        return d;
    }

    Diagnostic TransactionValidator::MakeFieldDiag(
        SEVERITY sev, RULE_VIOLATION code, const FieldRule& fr,
        uint32_t observed, uint32_t min, uint32_t max, std::string msg) {
        Diagnostic d{};
        d.severity = sev;
        d.code = code;
        d.record_type = fr.record_type;
        d.field_id = fr.field_id;
        d.observed = observed;
        d.expected_min = min;
        d.expected_max = max;
        d.message = std::move(msg);
        return d;
    }

    Diagnostic TransactionValidator::MakeSubfieldDiag(
        SEVERITY sev, RULE_VIOLATION code, const SubfieldRule& sfr,
        uint32_t observed, uint32_t min, uint32_t max, std::string msg) {
        Diagnostic d{};
        d.severity = sev;
        d.code = code;
        d.observed = observed;
        d.expected_min = min;
        d.expected_max = max;
        d.message = std::move(msg);
        return d;
    }
    Diagnostic TransactionValidator::MakeItemDiag(
        SEVERITY sev, RULE_VIOLATION code, const ItemRule& ir,
        uint32_t observed, uint32_t min, uint32_t max, std::string msg) {
        Diagnostic d{};
        d.severity = sev;
        d.code = code;
        d.observed = observed;
        d.expected_min = min;
        d.expected_max = max;
        d.message = std::move(msg);
        return d;
    }

    void TransactionValidator::UnknownRecord(
        const TransactionRule& tx, const uint16_t record_type,
        std::vector<Diagnostic>& out) const {
        SEVERITY s = PolicyToSeverity(tx.unknown_field_policy);
        Diagnostic d{};
        d.severity = s;
        d.code = RULE_VIOLATION::UNKNOWN_RECORD_PRESENT;
        d.record_type = record_type;
        d.message = "Unknown Record";
        out.push_back(d);
    }

    void TransactionValidator::UnknownField(
        const TransactionRule& tx, const uint16_t record_type,
        const uint16_t field, std::vector<Diagnostic>& out) const {
        SEVERITY s = PolicyToSeverity(tx.unknown_field_policy);
        Diagnostic d{};
        d.severity = s;
        d.code = RULE_VIOLATION::UNKNOWN_RECORD_PRESENT;
        d.record_type = record_type;
        d.field_id = field;
        d.message = "Unknown Field";
        out.push_back(d);
    }

}  // namespace OpenEFT
