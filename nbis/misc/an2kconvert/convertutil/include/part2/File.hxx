#ifndef PART2_FILE_HXX
#define PART2_FILE_HXX

#include "part1/File.hxx"
#include "part2/InformationRecord.hxx"
#include "part2/Record.hxx"
#include "part2/XMLElement.hxx"
#include "validate/Validation.hxx"

#include <list>
#include <memory>
#include <ostream>
#include <string>

namespace convert {
	namespace part1 {
		class File;
	}

	namespace part2 {
		using namespace std;

		class File {
		public:
			explicit File(string const& fileName, ValidationLevel vl, bool ebts);
			explicit File(part1::File const& part1File, bool ebts);
			InformationRecord const& getInfoRecord() const;
			list<Record*> const& getRecords() const;
			void validate() const;
			void writeFile(string const& fileName) const;
			static void printTree(XMLElement const& elem, int depth, ostream& ostr);
			~File();

		private:
			auto_ptr<XMLElement> readFile(string const& fileName) const;

			auto_ptr<InformationRecord> infoRecord;
			list<Record*> records;
		};
	}
}

#endif
