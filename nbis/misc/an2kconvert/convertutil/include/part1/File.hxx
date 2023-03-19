#ifndef PART1_FILE_HXX
#define PART1_FILE_HXX

#include "RecordType.hxx"
#include "part1/InformationRecord.hxx"
#include "part1/Record.hxx"
#include "part2/File.hxx"
#include "validate/Validation.hxx"

#include <list>
#include <memory>
#include <set>
#include <string>

namespace convert {
	namespace part2 {
		class File;
	}

	namespace part1 {
		using namespace std;

		const RecordType taggedRecordTypesArray[10] = {
			TYPE1, TYPE2, TYPE9, TYPE10, TYPE13, TYPE14, TYPE15, TYPE16, TYPE17, TYPE99
		};
		const set<RecordType> taggedRecordTypes(taggedRecordTypesArray, taggedRecordTypesArray + 10);

		const RecordType binaryImageRecordTypesArray[4] = {
			TYPE3, TYPE4, TYPE5, TYPE6
		};
		const set<RecordType> binaryImageRecordTypes(binaryImageRecordTypesArray, binaryImageRecordTypesArray + 4);

		const RecordType binaryUserDefinedRecordTypesArray[1] = {
			TYPE7
		};
		const set<RecordType> binaryUserDefinedRecordTypes(binaryUserDefinedRecordTypesArray, binaryUserDefinedRecordTypesArray + 1);

		const RecordType binarySignatureTypesArray[1] = {
			TYPE8
		};
		const set<RecordType> binarySignatureRecordTypes(binarySignatureTypesArray, binarySignatureTypesArray + 1);

		class File {
		public:
			explicit File(string const& fileName, ValidationLevel vl, bool ebts);
			explicit File(part2::File const& part2File);
			InformationRecord const& getInfoRecord() const;
			list<Record*> const& getRecords() const;
			void validate() const;
			void writeFile(string const& fileName);
			~File();

		private:
			auto_ptr<string> readFile(string const& fileName) const;
			auto_ptr<string> toBytesForFile();

			auto_ptr<InformationRecord> infoRecord;
			list<Record*> records;
		};
	}
}

#endif
