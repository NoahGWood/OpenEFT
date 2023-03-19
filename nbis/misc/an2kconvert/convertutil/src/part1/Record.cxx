#include "part1/Record.hxx"
#include "part1/RecordValidator.hxx"
#include "convert/RecordConverter.hxx"
#include "utils.hxx"

namespace convert {
	namespace part1 {
		using namespace std;
		using namespace convert::part2;

		Record::Record(RecordType recordType)
		: recordType(recordType), recordVariant(TYPE_UNKNOWN) {}

		Record::Record(part2::Record const& part2Record)
		: recordType(part2Record.getRecordType()), recordVariant(part2Record.getRecordVariant()) {
			RecordConverter::convertRecord(part2Record, *this);
		}

		Record::~Record() {
			deleteContents<Field*>(fields);
		}

		RecordType Record::getRecordType() const {
			return recordType;
		}

		RecordVariant Record::getRecordVariant() const {
			return recordVariant;
		}

		list<Field*> const& Record::getFields() const {
			return fields;
		}

		bool Record::hasField(FieldID const& fieldId) const {
			return findField(fieldId) != NULL;
		}

		Field const& Record::getField(FieldID const& fieldId) const {
			Field* field = findField(fieldId);
			if(field == NULL) {
				throw logic_error("Part1Record.getField(): Field does not exist");
			}
			return *field;
		}

		Field& Record::getField(FieldID const& fieldId) {
			Field* field = findField(fieldId);
			if(field == NULL) {
				throw logic_error("Part1Record.getField(): Field does not exist");
			}
			return *field;
		}

		void Record::addField(auto_ptr<Field> field) {
			for(list<Field*>::iterator it = fields.begin(); it != fields.end(); it++) {
				if((*it)->getFieldID() == field->getFieldID()) {
					throw logic_error("Part1Record.addField(): Field already exists");
					//field already in list
				}
				if((*it)->getFieldID() > field->getFieldID()) {
					fields.insert(it, field.release());
					break;
				}
			}
			if(field.get() != NULL) {
				fields.push_back(field.release());
			}
		}

		void Record::removeField(FieldID const& fieldId) {
			for(list<Field*>::iterator it = fields.begin(); it != fields.end(); it++) {
				if((*it)->getFieldID() == fieldId) {
					delete *it;
					fields.erase(it);
					break;
				}
			}
		}

		void Record::validate(ValidationLevel vl, bool ebts) {
			recordVariant = RecordValidator::validateRecord(*this, vl, ebts);
		}

		Field* Record::findField(FieldID const& fieldId) const {
			for(list<Field*>::const_iterator it = fields.begin(); it != fields.end(); it++) {
				Field* field = *it;
				if(field->getFieldID() == fieldId) {
					return field;
				}
			}
			return NULL;
		}
	}
}
