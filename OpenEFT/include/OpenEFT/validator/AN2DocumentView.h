// Copyright (c) 2026 Noah G. Wood
// SPDX-License-Identifier: LicenseRef-OpenEFT
//
// This file is part of OpenEFT.
// OpenEFT is source-available software. See LICENSE file for details.
#pragma once
#include <OpenEFT/validator/DocumentView.h>

namespace OpenEFT {
    class AN2DocumentView : public DocumentView {
      public:
        explicit AN2DocumentView(const nnist::File& f) : m_File(f) {}

        std::vector<nnist::Record> Records() const override {
            return m_File.records;
        }
        std::vector<nnist::Field> Fields(uint16_t record) const override {
            for (auto& r : m_File.records) {
                if (r.type == record) {
                    return r.fields;
                }
            }
            return {};
        }
        uint32_t RecordCount(uint16_t record_type) const override;
        uint32_t FieldCount(uint16_t record_type,
                            uint16_t field_id) const override;

        void ForEachFieldInstance(
            uint16_t record_type, uint16_t field_id,
            const std::function<void(FieldInstanceView)>& fn) const override;

      private:
        const nnist::File& m_File;
    };
}  // namespace OpenEFT
