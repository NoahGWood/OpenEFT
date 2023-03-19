#ifndef BINARYIMAGERECORD_HXX
#define BINARYIMAGERECORD_HXX

#include "RecordType.hxx"
#include "part1/BinaryRecord.hxx"
#include "part2/Record.hxx"

#include <string>
#include <vector>

namespace convert {
	namespace part1 {
		using namespace std;

		class BinaryImageRecord : public BinaryRecord {
		public:
			BinaryImageRecord(RecordType recordType, string& bytes, ValidationLevel vl, bool ebts);
			explicit BinaryImageRecord(part2::Record const& part2Record);

		private:
			vector<BinaryFieldDesc> const& getFieldDescs() const;
			static vector<BinaryFieldDesc> const& getFieldDescs(string const& bytes);

			static const BinaryFieldDesc type3_6FieldsArray[9];
			static const vector<BinaryFieldDesc> type3_6Fields;
		};
	}
}

#endif
