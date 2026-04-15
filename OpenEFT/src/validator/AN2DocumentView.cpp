#include <OpenEFT/validator/AN2DocumentView.h>
#include <OpenEFT/validator/Helpers.h>

namespace OpenEFT {
    uint32_t AN2DocumentView::RecordCount(uint16_t record_type) const {
        uint32_t count = 0;
        for (const auto& r : m_File.records)
            if (r.type == record_type)
                ++count;
        return count;
    }

    uint32_t AN2DocumentView::FieldCount(uint16_t record_type,
                                         uint16_t field_id) const {
        uint32_t count = 0;

        for (const auto& r : m_File.records) {
            if (r.type != record_type)
                continue;

            for (const auto& f : r.fields) {
                if (ParseFieldId(f.tag) == field_id)
                    ++count;
            }
        }

        return count;
    }

    void AN2DocumentView::ForEachFieldInstance(
        uint16_t record_type, uint16_t field_id,
        const std::function<void(FieldInstanceView)>& fn) const {
        for (uint32_t ri = 0; ri < m_File.records.size(); ++ri) {
            const auto& r = m_File.records[ri];
            if (r.type != record_type)
                continue;
            for (uint32_t fi = 0; fi < r.fields.size(); ++fi) {
                const auto& f = r.fields[fi];

                if (ParseFieldId(f.tag) != field_id)
                    continue;

                fn(FieldInstanceView{&r, &f, ri, fi});
            }
        }
    }
}  // namespace OpenEFT
