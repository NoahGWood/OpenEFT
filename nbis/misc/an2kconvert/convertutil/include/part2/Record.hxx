#ifndef PART2_RECORD_HXX
#define PART2_RECORD_HXX

#include "RecordType.hxx"
#include "part1/Record.hxx"
#include "part2/XMLElement.hxx"
#include "validate/Validation.hxx"

#include <memory>

namespace convert {
	namespace part1 {
		class Record;
	}

	namespace part2 {
		using namespace std;

		class Record {
		public:
			Record(RecordType recordType, auto_ptr<XMLElement> recordRoot, ValidationLevel vl, bool ebts);
			explicit Record(part1::Record const& part1Record);
			RecordType getRecordType() const;
			RecordVariant getRecordVariant() const;
			XMLElement const& getRootElement() const;
			XMLElement& getRootElement();
			void setRootElement(auto_ptr<XMLElement> elem);


		protected:
			void validate(ValidationLevel vl, bool ebts);

			RecordType recordType;
			RecordVariant recordVariant;
			auto_ptr<XMLElement> rootElement;
		};
	}
}
#endif
