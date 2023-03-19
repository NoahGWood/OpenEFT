#include "ITLPackage.hxx"

namespace convert {
	using namespace std;

	bool isPart1(FileType fileType) {
		return fileType == AN_PART1 || fileType == EBTS_PART1;
	}

	bool isPart2(FileType fileType) {
		return fileType == AN_PART2 || fileType == EBTS_PART2;
	}

	bool isEBTS(FileType fileType) {
		return fileType == EBTS_PART1 || fileType == EBTS_PART2;
	}

	ITLPackage::ITLPackage(string const& fileName, FileType fileType, ValidationLevel vl)
	: ebts(isEBTS(fileType)) {
		if(isPart1(fileType)) {
			part1File = auto_ptr<part1::File>(new part1::File(fileName, vl, isEBTS(fileType)));
		}
		if(isPart2(fileType)) {
			part2File = auto_ptr<part2::File>(new part2::File(fileName, vl, isEBTS(fileType)));
		}
	}

	void ITLPackage::outputPart1File(string const& fileName) {
		if(part1File.get() == NULL) {
			convertPart2File();
		}
		part1File->writeFile(fileName);
	}

	void ITLPackage::outputPart2File(string const& fileName) {
		if(part2File.get() == NULL) {
			convertPart1File();
		}
		part2File->writeFile(fileName);
	}

	void ITLPackage::convertPart1File() {
		part2File = auto_ptr<part2::File>(new part2::File(*part1File, ebts));
	}

	void ITLPackage::convertPart2File() {
		part1File = auto_ptr<part1::File>(new part1::File(*part2File));
	}
}
