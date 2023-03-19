#include "part1/File.hxx"
#include "part1/BinaryImageRecord.hxx"
#include "part1/BinarySignatureRecord.hxx"
#include "part1/BinaryUserDefinedRecord.hxx"
#include "Errors.hxx"
#include "utils.hxx"

#include <fstream>
#include <iostream>
#include <iterator>

namespace convert {
	namespace part1 {
		using namespace std;

		File::File(string const& fileName, ValidationLevel vl, bool ebts) {
			auto_ptr<string> bytes = readFile(fileName);
			infoRecord = auto_ptr<InformationRecord>(new InformationRecord(*bytes, vl, ebts));
			list<FileContents> fileContents = infoRecord->getFileContents();
			for(list<FileContents>::const_iterator it = fileContents.begin(); it != fileContents.end(); it++) {
				FileContents const& fc = *it;
				if(binaryImageRecordTypes.count(fc.recordType) != 0) {
					records.push_back(new BinaryImageRecord(fc.recordType, *bytes, vl, ebts));
				}
				if(binarySignatureRecordTypes.count(fc.recordType) != 0) {
					records.push_back(new BinarySignatureRecord(fc.recordType, *bytes, vl, ebts));
				}
				if(binaryUserDefinedRecordTypes.count(fc.recordType) != 0) {
					records.push_back(new BinaryUserDefinedRecord(fc.recordType, *bytes, vl, ebts));
				}
				if(taggedRecordTypes.count(fc.recordType) != 0) {
					records.push_back(new TaggedRecord(fc.recordType, *bytes, vl, ebts));
				}
			}
			if(bytes->size() != 0) {
				throw ParseError("data after last record");
			}
		}

		File::File(part2::File const& part2File) {
			infoRecord = auto_ptr<InformationRecord>(new InformationRecord(part2File.getInfoRecord()));
			list<part2::Record*> const& part2Records = part2File.getRecords();
			for(list<part2::Record*>::const_iterator it = part2Records.begin(); it != part2Records.end(); it++) {
				part2::Record const& part2Record = *(*it);
				if(binaryImageRecordTypes.count(part2Record.getRecordType()) != 0) {
					records.push_back(new BinaryImageRecord(part2Record));
				}
				if(binarySignatureRecordTypes.count(part2Record.getRecordType()) != 0) {
					records.push_back(new BinarySignatureRecord(part2Record));
				}
				if(binaryUserDefinedRecordTypes.count(part2Record.getRecordType()) != 0) {
					records.push_back(new BinaryUserDefinedRecord(part2Record));
				}
				if(taggedRecordTypes.count(part2Record.getRecordType()) != 0) {
					records.push_back(new TaggedRecord(part2Record));
				}
			}
		}

		InformationRecord const& File::getInfoRecord() const {
			return *infoRecord;
		}

		list<Record*> const& File::getRecords() const {
			return records;
		}

		void File::validate() const {

		}

		void File::writeFile(string const& fileName) {
			auto_ptr<string> bytes = toBytesForFile();
			ofstream out(fileName.c_str(), ifstream::out|ifstream::binary|ifstream::trunc);
			if(out.fail()) {
				throw IOError("Could not open output file");
				//file failed to open properly
			}
			copy(bytes->begin(), bytes->end(), ostreambuf_iterator<char>(out.rdbuf()));
			out.close();
		}

		File::~File() {
			deleteContents<Record*>(records);
		}

		auto_ptr<string> File::readFile(string const& fileName) const {
			ifstream in(fileName.c_str(), ifstream::in|ifstream::binary);
			if(in.fail()) {
				throw IOError("Could not open input file");
				//file failed to open properly, file may not exist
			}
			auto_ptr<string> bytes(new string(istreambuf_iterator<char>(in.rdbuf()), istreambuf_iterator<char>()));
			in.close();
			return bytes;
		}

		auto_ptr<string> File::toBytesForFile() {
			auto_ptr<string> fileStr(new string(*(infoRecord->toBytesForFile())));
			for(list<Record*>::iterator it = records.begin(); it != records.end(); it++) {
				Record& part1Record = *(*it);
				*fileStr += *(part1Record.toBytesForFile());
			}
			return fileStr;
		}
	}
}
