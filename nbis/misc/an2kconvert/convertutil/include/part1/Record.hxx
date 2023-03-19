#ifndef PART1_RECORD_HXX
#define PART1_RECORD_HXX

#include "part1/Field.hxx"
#include "part1/FieldID.hxx"
#include "part2/Record.hxx"
#include "validate/Validation.hxx"
#include "Errors.hxx"
#include "RecordType.hxx"

#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>

namespace convert {
	namespace part2 {
		class Record;
	}

	namespace part1 {
		using namespace std;

		class Record {
		public:
			explicit Record(RecordType recordType);
			explicit Record(part2::Record const& part2Record);
			~Record();
			RecordType getRecordType() const;
			RecordVariant getRecordVariant() const;
			list<Field*> const& getFields() const;
			bool hasField(FieldID const& fieldId) const;
			Field const& getField(FieldID const& fieldId) const;
			Field& getField(FieldID const& fieldId);
			void addField(auto_ptr<Field> field);
			void removeField(FieldID const& fieldId);
			virtual auto_ptr<string> toBytesForFile() = 0;

		protected:
			void validate(ValidationLevel vl, bool ebts);
			Field* findField(FieldID const& fieldId) const;

			RecordType recordType;
			RecordVariant recordVariant;
			list<Field*> fields;
		};
	}
}

#endif
