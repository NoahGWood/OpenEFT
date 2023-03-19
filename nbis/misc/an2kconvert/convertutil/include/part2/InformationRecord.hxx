#ifndef PART2_INFORMATIONRECORD_HXX
#define PART2_INFORMATIONRECORD_HXX

#include "FileContents.hxx"
#include "part1/InformationRecord.hxx"
#include "part2/Record.hxx"
#include "part2/XMLElement.hxx"
#include "validate/Validation.hxx"

#include <list>
#include <memory>
#include <string>

namespace convert {
	namespace part1 {
		class InformationRecord;
	}

	namespace part2 {
		using namespace std;

		class InformationRecord : public Record {
		public:
			explicit InformationRecord(auto_ptr<XMLElement> recordRoot, ValidationLevel vl, bool ebts);
			explicit InformationRecord(part1::InformationRecord const& part1Record, bool ebts);
			list<FileContents> const& getFileContents() const;

		private:
			void findFileContents(bool ebts);

			list<FileContents> fileContents;
		};
	}
}

#endif
