#include "part2/Record.hxx"
#include "convert/RecordConverter.hxx"
#include "part2/RecordValidator.hxx"

#include <iostream>

namespace convert {
	namespace part2 {
		using namespace std;

		Record::Record(RecordType recordType, auto_ptr<XMLElement> recordRoot, ValidationLevel vl, bool ebts)
		: recordType(recordType), recordVariant(TYPE_UNKNOWN), rootElement(recordRoot) {
			validate(vl, ebts);
		}

		Record::Record(part1::Record const& part1Record)
		: recordType(part1Record.getRecordType()), recordVariant(part1Record.getRecordVariant()), rootElement(NULL) {
			RecordConverter::convertRecord(part1Record, *this);
		}

		RecordType Record::getRecordType() const {
			return recordType;
		}

		RecordVariant Record::getRecordVariant() const {
			return recordVariant;
		}

		XMLElement const& Record::getRootElement() const {
			return *rootElement;
		}

		XMLElement& Record::getRootElement() {
			return *rootElement;
		}

		void Record::setRootElement(auto_ptr<XMLElement> elem) {
			rootElement = elem;
		}

		void Record::validate(ValidationLevel vl, bool ebts) {
			recordVariant = RecordValidator::validateRecord(*this, vl, ebts);
		}
	}
}
