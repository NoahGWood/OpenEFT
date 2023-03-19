#ifndef FILECONTENTS_HXX
#define FILECONTENTS_HXX

#include "RecordType.hxx"

namespace convert {
	using namespace std;

	struct FileContents {
		RecordType recordType;
		int identificationChar;
	};
}

#endif
