#ifndef BINARYRECORD_HXX
#define BINARYRECORD_HXX

#include "RecordType.hxx"
#include "part1/FieldID.hxx"
#include "part1/Record.hxx"
#include "part2/Record.hxx"
#include "validate/Validation.hxx"

#include <memory>
#include <string>
#include <vector>

namespace convert {
	namespace part1 {
		using namespace std;

		struct BinaryFieldDesc {
			FieldType fieldType;
			int subfieldsCount;
		};

		class BinaryRecord : public Record {
		public:
			BinaryRecord(RecordType recordType, string& bytes, vector<BinaryFieldDesc> const& (*getFieldDescs)(string const&), ValidationLevel vl, bool ebts);
			explicit BinaryRecord(part2::Record const& part2Record);
			auto_ptr<string> toBytesForFile();

		private:
			virtual vector<BinaryFieldDesc> const& getFieldDescs() const = 0;
			size_t findLengthField(string const& bytes) const;
			void findFields(string const& bytes, vector<BinaryFieldDesc> const& (*getFieldDescs)(string const&));
			size_t findNextField(string const& bytes, size_t startIndex, FieldID const& fieldId, BinaryFieldDesc desc);
			void calculateLength();
		};
	}
}

#endif
