#ifndef TAGGEDRECORD_HXX
#define TAGGEDRECORD_HXX

#include "RecordType.hxx"
#include "part1/Record.hxx"
#include "part2/Record.hxx"
#include "validate/Validation.hxx"

#include <memory>
#include <string>

namespace convert {
	namespace part1 {
		using namespace std;

		class TaggedRecord : public Record {
		public:
			TaggedRecord(RecordType recordType, string& bytes, ValidationLevel vl, bool ebts);
			explicit TaggedRecord(part2::Record const& part2Record);
			auto_ptr<string> toBytesForFile();

		private:
			size_t findLengthField(string const& bytes) const;
			void findFields(string const& bytes);
			size_t findNextField(string const& bytes, size_t startIndex);
			void calculateLength();
		};
	}
}

#endif
