#include "part2/InformationRecord.hxx"
#include "utils.hxx"

#include <iomanip>
#include <sstream>

namespace convert {
	namespace part2 {
		using namespace std;

		InformationRecord::InformationRecord(auto_ptr<XMLElement> recordRoot, ValidationLevel vl, bool ebts)
		: Record(TYPE1, recordRoot, vl, ebts) {
			findFileContents(ebts);
		}

		InformationRecord::InformationRecord(part1::InformationRecord const& part1Record, bool ebts)
		: Record(part1Record) {
			findFileContents(ebts);
		}

		list<FileContents> const& InformationRecord::getFileContents() const {
			return fileContents;
		}

		void InformationRecord::findFileContents(bool ebts) {
			XMLElement const& transaction = rootElement->findChild(ElementID("Transaction", ebts ? EBTS : AN));
			XMLElement const& contentSummary = transaction.findChild(ElementID("TransactionContentSummary", AN));
			XMLElement const& recordCount = contentSummary.findChild(ElementID("ContentRecordCount", AN));
			if(recordCount == XMLElement::MISSING_ELEMENT) {
				throw ValidationError("Type 1 record has no <ansi-nist:ContentRecordCount> element");
				//required element is missing
			}
			int numberOfRecords = stringToInt(recordCount.getText());
			if(contentSummary.getChildren().size() - 2 != numberOfRecords) {
				throw ValidationError("Type 1 record <ansi-nist:ContentRecordCount> has incorrect record count");
				//number of records in file is incorrect
			}
			for(list<XMLElement*>::const_iterator it = contentSummary.getChildren().begin(); it != contentSummary.getChildren().end(); it++) {
				XMLElement const& elem = *(*it);
				if(elem.getElementId().getElementName() == "ContentFirstRecordCategoryCode" || elem.getElementId().getElementName() == "ContentRecordCount") {
					continue;
				} else if(elem.getElementId().getElementName() != "ContentRecordSummary") {
					throw ValidationError("Type 1 record has invalid element in <ansi-nist:TransactionContentSummary>");
					//invalid element
				}
				XMLElement const& imageRefId = elem.findChild(ElementID("ImageReferenceIdentification", AN));
				XMLElement const& id = imageRefId.findChild(ElementID("IdentificationID", NC));
				XMLElement const& recordCategoryCode = elem.findChild(ElementID("RecordCategoryCode", AN));
				if(id == XMLElement::MISSING_ELEMENT || recordCategoryCode == XMLElement::MISSING_ELEMENT) {
					throw ValidationError("Type 1 record is missing <ansi-nist:RecordCategoryCode> in <ansi-nist:ContentRecordSummary> element");
					//required element is missing
				}
				FileContents fc;
				fc.recordType = findRecordType(recordCategoryCode.getText());
				fc.identificationChar = stringToInt(id.getText());
				fileContents.push_back(fc);
			}
		}
	}
}
