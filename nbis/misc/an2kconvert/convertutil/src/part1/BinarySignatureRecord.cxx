#include "part1/BinarySignatureRecord.hxx"
#include "Errors.hxx"
#include "utils.hxx"

#include <iostream>

namespace convert {
	namespace part1 {
		using namespace std;

		BinarySignatureRecord::BinarySignatureRecord(RecordType recordType, string& bytes, ValidationLevel vl, bool ebts)
		: BinaryRecord(recordType, bytes, &BinarySignatureRecord::getFieldDescs, vl, ebts) {}

		BinarySignatureRecord::BinarySignatureRecord(part2::Record const& part2Record)
		: BinaryRecord(part2Record) {}

		vector<BinaryFieldDesc> const& BinarySignatureRecord::getFieldDescs() const {
			if(recordVariant == TYPE8_A) {
				return type8aFields;
			} else if(recordVariant == TYPE8_B) {
				return type8bFields;
			} else {
				throw logic_error("BinarySignatureRecord.getFieldDescs(): invalid RecordVariant");
			}
		}

		vector<BinaryFieldDesc> const& BinarySignatureRecord::getFieldDescs(string const& bytes) {
			if(bytes.size() < 6) {
				throw ValidationError("Type 8 record has no SRT field (8.004)");
				//SRT field is missing
			}
			Field field(FieldID::TYPE_8_SRT, string(bytes, 6, 1), BinaryU8Field);
			int srtValue = stringToInt(field.getSubfield(0).getItem(0).toString());
			if(srtValue == 0 || srtValue == 1) {
				return type8aFields;
			} else if(srtValue = 2) {
				return type8bFields;
			} else {
				throw ValidationError("Type 8 record has invalid value in SRT field (8.004)");
				//invalid srtValue
			}
		}

		const BinaryFieldDesc BinarySignatureRecord::type8aFieldsArray[8] = {
			{BinaryU32Field, 1},
			{BinaryU8Field, 1},
			{BinaryU8Field, 1},
			{BinaryU8Field, 1},
			{BinaryU8Field, 1},
			{BinaryU16Field, 1},
			{BinaryU16Field, 1},
			{BinaryImageField, 1}
		};
		const vector<BinaryFieldDesc> BinarySignatureRecord::type8aFields(BinarySignatureRecord::type8aFieldsArray, BinarySignatureRecord::type8aFieldsArray + 8);

		const BinaryFieldDesc BinarySignatureRecord::type8bFieldsArray[8] = {
			{BinaryU32Field, 1},
			{BinaryU8Field, 1},
			{BinaryU8Field, 1},
			{BinaryU8Field, 1},
			{BinaryU8Field, 1},
			{BinaryU16Field, 1},
			{BinaryU16Field, 1},
			{BinaryVectorField, 1}
		};
		const vector<BinaryFieldDesc> BinarySignatureRecord::type8bFields(BinarySignatureRecord::type8bFieldsArray, BinarySignatureRecord::type8bFieldsArray + 8);
	}
}
