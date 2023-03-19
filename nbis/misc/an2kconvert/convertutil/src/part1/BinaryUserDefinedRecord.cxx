#include "part1/BinaryUserDefinedRecord.hxx"

namespace convert {
	namespace part1 {
		using namespace std;

		BinaryUserDefinedRecord::BinaryUserDefinedRecord(RecordType recordType, string& bytes, ValidationLevel vl, bool ebts)
		: BinaryRecord(recordType, bytes, &BinaryUserDefinedRecord::getFieldDescs, vl, ebts) {}

		BinaryUserDefinedRecord::BinaryUserDefinedRecord(part2::Record const& part2Record)
		: BinaryRecord(part2Record) {}

		vector<BinaryFieldDesc> const& BinaryUserDefinedRecord::getFieldDescs() const {
			return type7Fields;
		}

		vector<BinaryFieldDesc> const& BinaryUserDefinedRecord::getFieldDescs(string const& bytes) {
			return type7Fields;
		}

		const BinaryFieldDesc BinaryUserDefinedRecord::type7FieldsArray[2] = {
			{BinaryU32Field, 1},
			{BinaryU8Field, 1}
		};

		const vector<BinaryFieldDesc> BinaryUserDefinedRecord::type7Fields(BinaryUserDefinedRecord::type7FieldsArray, BinaryUserDefinedRecord::type7FieldsArray + 2);
	}
}
