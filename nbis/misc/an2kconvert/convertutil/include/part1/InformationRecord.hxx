#ifndef PART1_INFORMATIONRECORD_HXX
#define PART1_INFORMATIONRECORD_HXX

#include "FileContents.hxx"
#include "part1/TaggedRecord.hxx"
#include "part2/InformationRecord.hxx"

#include <list>
#include <string>

namespace convert {
	namespace part2 {
		class InformationRecord;
	}

	namespace part1 {
		using namespace std;

		class InformationRecord : public TaggedRecord {
		public:
			explicit InformationRecord(string& bytes, ValidationLevel vl, bool ebts);
			explicit InformationRecord(part2::InformationRecord const& part2Record);
			list<FileContents> const& getFileContents();

		private:
			void findFileContents();

			list<FileContents> fileContents;
		};
	}
}

#endif
