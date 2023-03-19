#ifndef BINARYSIGNATURERECORD_HXX
#define BINARYSIGNATURERECORD_HXX

#include "RecordType.hxx"
#include "part1/BinaryRecord.hxx"
#include "part2/Record.hxx"

#include <string>
#include <vector>

namespace convert {
	namespace part1 {
		using namespace std;

		class BinarySignatureRecord : public BinaryRecord {
		public:
			BinarySignatureRecord(RecordType recordType, string& bytes, ValidationLevel vl, bool ebts);
			explicit BinarySignatureRecord(part2::Record const& part2Record);

		private:
			vector<BinaryFieldDesc> const& getFieldDescs() const;
			static vector<BinaryFieldDesc> const& getFieldDescs(string const& bytes);

			static const BinaryFieldDesc type8aFieldsArray[8];
			static const vector<BinaryFieldDesc> type8aFields;
			static const BinaryFieldDesc type8bFieldsArray[8];
			static const vector<BinaryFieldDesc> type8bFields;
		};
	}
}

#endif
