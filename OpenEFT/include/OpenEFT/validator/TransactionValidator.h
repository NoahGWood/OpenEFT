// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <OpenEFT/validator/Data.h>
#include <OpenEFT/validator/DocumentView.h>
#include <OpenEFT/rules/Rules.h>

namespace OpenEFT {
    struct Options {
        bool validate_lengths = true;
        bool validate_occurs = true;
        bool validate_unknown_records = false;
        bool validate_unknown_fields = false;
    };

    class TransactionValidator {
      public:
        explicit TransactionValidator(const Options& opt = {}) : m_Opt(opt) {}

        std::vector<Diagnostic> Validate(const TransactionRule& tx,
                                         const DocumentView& doc) const;
      private:
        Options m_Opt;

        void ValidateRecordSets(const TransactionRule& tx,
                                const DocumentView& doc,
                                std::vector<Diagnostic>& out) const;
        void ValidateRecord(const RecordRule& rr, const DocumentView& doc,
                            std::vector<Diagnostic>& out) const;
        void ValidateField(const FieldRule& fr, const DocumentView& doc,
                           std::vector<Diagnostic>& out) const;
        void ValidateFieldInstance(const FieldRule& fr,
                                   const FieldInstanceView& view,
                                   std::vector<Diagnostic>& out) const;
        void ValidateSubfield(const SubfieldRule& sfr,
                              const nnist::Field& field,
                              std::vector<Diagnostic>& out) const;
        void ValidateItem(const ItemRule& rule, const nnist::Item& view,
                          std::vector<Diagnostic>& out) const;
        bool ValidateItemType(const ItemRule& rule,
                              const nnist::Item& view,
                              std::vector<Diagnostic>& out) const;
        void ValidateItemConstraints(const ItemRule& rule,
                                     const nnist::Item& view,
                                     std::vector<Diagnostic>& out) const;
        // void ValidateUnknownStructure() const;
        void UnknownRecord(const TransactionRule& tx,
                           const uint16_t record_type,
                           std::vector<Diagnostic>& out) const;
        void UnknownField(const TransactionRule& tx,
                          const uint16_t record_type, const uint16_t field,
                          std::vector<Diagnostic>& out) const;

        static SEVERITY PolicyToSeverity(UNKNOWN_FIELD_POLICY p);
        static Diagnostic MakeRecordSetDiag(SEVERITY sev, RULE_VIOLATION code,
                                            uint32_t total, uint32_t min,
                                            uint32_t max);
        static Diagnostic MakeRecordDiag(SEVERITY sev, RULE_VIOLATION code,
                                         uint16_t rt, uint32_t observed,
                                         uint32_t min, uint32_t max,
                                         std::string msg);
        static Diagnostic MakeFieldDiag(SEVERITY sev, RULE_VIOLATION code,
                                        const FieldRule& fr,
                                        uint32_t observed, uint32_t min,
                                        uint32_t max, std::string msg);
        static Diagnostic MakeSubfieldDiag(SEVERITY sev, RULE_VIOLATION code,
                                           const SubfieldRule& sfr,
                                           uint32_t observed, uint32_t min,
                                           uint32_t max, std::string msg);
        static Diagnostic MakeItemDiag(SEVERITY sev, RULE_VIOLATION code,
                                       const ItemRule& ir,
                                       uint32_t observed, uint32_t min,
                                       uint32_t max, std::string msg);
    };
}  // namespace OpenEFT
