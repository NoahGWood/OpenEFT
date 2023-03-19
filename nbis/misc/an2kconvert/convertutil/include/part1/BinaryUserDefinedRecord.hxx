#ifndef BINARYUSERDEFINEDRECORD_HXX
#define BINARYUSERDEFINEDRECORD_HXX

#include "RecordType.hxx"
#include "part1/BinaryRecord.hxx"
#include "part2/Record.hxx"

#include <string>
#include <vector>

namespace convert {
	namespace part1 {
		using namespace std;

		class BinaryUserDefinedRecord : public BinaryRecord {
		public:
			BinaryUserDefinedRecord(RecordType recordType, string& bytes, ValidationLevel vl, bool ebts);
			explicit BinaryUserDefinedRecord(part2::Record const& part2Record);

		private:
			vector<BinaryFieldDesc> const& getFieldDescs() const;
			static vector<BinaryFieldDesc> const& getFieldDescs(string const& bytes);

			static const BinaryFieldDesc type7FieldsArray[2];
			static const vector<BinaryFieldDesc> type7Fields;
		};
	}
}
#endif
