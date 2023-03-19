#ifndef ITLPACKAGE_HXX
#define ITLPACKAGE_HXX

#include "part1/File.hxx"
#include "part2/File.hxx"
#include "validate/Validation.hxx"

#include <memory>
#include <string>

namespace convert {
	using namespace std;

	enum FileType {
		AN_PART1,
		AN_PART2,
		EBTS_PART1,
		EBTS_PART2
	};

	bool isPart1(FileType fileType);
	bool isPart2(FileType fileType);
	bool isEBTS(FileType fileType);

	class ITLPackage {
	public:
		ITLPackage(string const& fileName, FileType fileType, ValidationLevel vl);
		void outputPart1File(string const& fileName);
		void outputPart2File(string const& fileName);

	private:
		void convertPart1File();
		void convertPart2File();

		auto_ptr<part1::File> part1File;
		auto_ptr<part2::File> part2File;
		bool ebts;
	};
}

#endif
