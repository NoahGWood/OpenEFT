#include "part1/InformationRecord.hxx"
#include "Errors.hxx"
#include "utils.hxx"

#include <iostream>

namespace convert {
	namespace part1 {
		using namespace std;

		InformationRecord::InformationRecord(string& bytes, ValidationLevel vl, bool ebts)
		: TaggedRecord(TYPE1, bytes, vl, ebts) {
			findFileContents();
		}

		InformationRecord::InformationRecord(part2::InformationRecord const& part2Record)
		: TaggedRecord(part2Record) {
			findFileContents();
		}

		list<FileContents> const& InformationRecord::getFileContents() {
			return fileContents;
		}

		void InformationRecord::findFileContents() {
			if(!hasField(FieldID::TYPE_1_CNT)) {
				throw ValidationError("Type 1 record has no CNT field (1.003)");
			}
			Field const& fileContentsField = getField(FieldID::TYPE_1_CNT);
			int numberOfRecords = stringToInt(fileContentsField.getSubfield(0).getItem(1).toString());
			if(fileContentsField.subfieldsCount() - 1 != numberOfRecords) {
				throw ValidationError("Type 1 record CNT field (1.003) has incorrect record count");
			}
			for(int i = 1; i < fileContentsField.subfieldsCount(); i++) {
				if(fileContentsField.getSubfield(i).itemsCount() != 2) {
					throw ValidationError("Type 1 record CNT field (1.003) has a subfield with an incorrect number of items");
				}
				FileContents fc;
				fc.recordType = findRecordType(fileContentsField.getSubfield(i).getItem(0).toString());
				fc.identificationChar = stringToInt(fileContentsField.getSubfield(i).getItem(1).toString());
				fileContents.push_back(fc);
			}
		}
	}
}
