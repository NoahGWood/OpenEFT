#include "part1/BinaryRecord.hxx"
#include "Errors.hxx"
#include "utils.hxx"

#include <iostream>

namespace convert {
	namespace part1 {
		using namespace std;

		BinaryRecord::BinaryRecord(RecordType recordType, string& bytes, vector<BinaryFieldDesc> const& (*getFieldDescs)(string const&), ValidationLevel vl, bool ebts)
		: Record(recordType) {
			size_t recordLength = findLengthField(bytes);
			if(recordLength > bytes.size()) {
				throw ParseError("Binary record has an incorrect length field value");
				//recordLength is greater than size of bytes
			}
			findFields(string(bytes.begin(), bytes.begin() + recordLength), getFieldDescs);
			bytes.erase(bytes.begin(), bytes.begin() + recordLength);
			validate(vl, ebts);
		}

		BinaryRecord::BinaryRecord(part2::Record const& part2Record)
		: Record(part2Record) {}

		auto_ptr<string> BinaryRecord::toBytesForFile() {
			calculateLength();
			auto_ptr<string> bytes(new string);
			vector<BinaryFieldDesc> const& fieldDescs = getFieldDescs();
			int fieldNumber = 1;
			for(vector<BinaryFieldDesc>::const_iterator it = fieldDescs.begin(); it != fieldDescs.end(); it++) {
				Field const& field = getField(FieldID(recordType, fieldNumber));
				*bytes += *(field.toBytesForFile(it->fieldType));
				fieldNumber++;
			}
			return bytes;
		}

		size_t BinaryRecord::findLengthField(string const& bytes) const {
			if(bytes.size() < 4) {
				throw ParseError("Binary record is less than 4 bytes long");
				//length field is missing
			}
			Field lenField(FieldID(recordType, 1), string(bytes, 0, 4), BinaryU32Field);
			return stringToSize_t(lenField.getSubfield(0).getItem(0).toString());
		}

		void BinaryRecord::findFields(string const& bytes, vector<BinaryFieldDesc> const& (*getFieldDescs)(string const&)) {
			//This is a function pointer and not a call to a virtual function
			//because C++ won't let you call virtual functions in constructors
			//or in methods called by a constructor.
			vector<BinaryFieldDesc> const& fieldDescs = (*getFieldDescs)(bytes);
			size_t startIndex = 0;
			int fieldNumber = 1;
			for(vector<BinaryFieldDesc>::const_iterator it = fieldDescs.begin(); it != fieldDescs.end(); it++) {
				startIndex = findNextField(bytes, startIndex, FieldID(recordType, fieldNumber), (*it));
				fieldNumber++;
			}
		}

		size_t BinaryRecord::findNextField(string const& bytes, size_t startIndex, FieldID const& fieldId, BinaryFieldDesc desc) {
			size_t fieldLen = 0;
			switch(desc.fieldType) {
			case BinaryImageField:
				fieldLen = bytes.size() - startIndex;
				break;
			case BinaryU8Field:
				fieldLen = desc.subfieldsCount;
				break;
			case BinaryU16Field:
				fieldLen = desc.subfieldsCount * 2;
				break;
			case BinaryU32Field:
				fieldLen = desc.subfieldsCount * 4;
				break;
			case BinaryVectorField:
				fieldLen = bytes.size() - startIndex;
				break;
			default:
				throw logic_error("BinaryRecord.findNextField(): invalid FieldType");
			}

			if(startIndex + fieldLen > bytes.size()) {
				throw ParseError("Binary record is missing fields");
				//ran out of data before reading all of the expected fields
			}
			fields.push_back(new Field(fieldId, string(bytes, startIndex, fieldLen), desc.fieldType));
			return startIndex + fieldLen;
		}

		void BinaryRecord::calculateLength() {
			removeField(FieldID(recordType, 1));

			size_t len = 0;
			vector<BinaryFieldDesc> const& fieldDescs = getFieldDescs();
			int fieldNumber = 1;
			for(vector<BinaryFieldDesc>::const_iterator it = fieldDescs.begin(); it != fieldDescs.end(); it++) {
				if(fieldNumber == 1) {
					fieldNumber++;
					continue;
				} else {
					Field const& field = getField(FieldID(recordType, fieldNumber));
					len += field.getLength(it->fieldType);
					fieldNumber++;
				}
			}

			Field dummyLen(FieldID(recordType, 1));
			dummyLen.addSubfield(size_tToString(len));
			len += dummyLen.getLength(BinaryU32Field);
			auto_ptr<Field> lenField(new Field(FieldID(recordType, 1)));
			lenField->addSubfield(intToString(len));
			addField(lenField);
		}
	}
}
