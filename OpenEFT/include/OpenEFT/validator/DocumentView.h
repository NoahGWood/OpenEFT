#pragma once
#include <nnist/nnist.h>
#include <OpenEFT/validator/Data.h>
#include <cstdint>
#include <functional>
#include <vector>

namespace OpenEFT {
    struct DocumentView {
        virtual ~DocumentView() = default;
        // record occurrences
        virtual std::vector<nnist::Record> Records() const = 0;
        // Field occurrences
        virtual std::vector<nnist::Field> Fields(
            uint16_t record_type) const = 0;
        virtual uint32_t RecordCount(uint16_t record_type) const = 0;
        // field occurrences within all records of this type
        // (Validator can either validate per-record instance, or aggregate.)
        virtual uint32_t FieldCount(uint16_t record_type,
                                    uint16_t field_id) const = 0;

        virtual void ForEachFieldInstance(
            uint16_t record_type, uint16_t field_id,
            const std::function<void(FieldInstanceView)>& fn) const = 0;
    };
}  // namespace OpenEFT
