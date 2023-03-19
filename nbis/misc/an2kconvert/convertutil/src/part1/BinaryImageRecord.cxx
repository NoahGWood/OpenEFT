#include "part1/BinaryImageRecord.hxx"

namespace convert {
	namespace part1 {
		using namespace std;

		BinaryImageRecord::BinaryImageRecord(RecordType recordType, string& bytes, ValidationLevel vl, bool ebts)
		: BinaryRecord(recordType, bytes, &BinaryImageRecord::getFieldDescs, vl, ebts) {}

		BinaryImageRecord::BinaryImageRecord(part2::Record const& part2Record)
		: BinaryRecord(part2Record) {}

		vector<BinaryFieldDesc> const& BinaryImageRecord::getFieldDescs() const {
			return type3_6Fields;
		}

		vector<BinaryFieldDesc> const& BinaryImageRecord::getFieldDescs(string const& bytes) {
			return type3_6Fields;
		}

		const BinaryFieldDesc BinaryImageRecord::type3_6FieldsArray[9] = {
			{BinaryU32Field, 1},
			{BinaryU8Field, 1},
			{BinaryU8Field, 1},
			{BinaryU8Field, 6},
			{BinaryU8Field, 1},
			{BinaryU16Field, 1},
			{BinaryU16Field, 1},
			{BinaryU8Field, 1},
			{BinaryImageField, 1}
		};

		const vector<BinaryFieldDesc> BinaryImageRecord::type3_6Fields(BinaryImageRecord::type3_6FieldsArray, BinaryImageRecord::type3_6FieldsArray + 9);
	}
}
