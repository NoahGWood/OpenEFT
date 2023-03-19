#ifndef RECORDTYPE_HXX
#define RECORDTYPE_HXX

#include <list>
#include <memory>
#include <string>

namespace convert {
	using namespace std;

	enum RecordType {
		TYPE1 = 1,
		TYPE2 = 2,
		TYPE3 = 3,
		TYPE4 = 4,
		TYPE5 = 5,
		TYPE6 = 6,
		TYPE7 = 7,
		TYPE8 = 8,
		TYPE9 = 9,
		TYPE10 = 10,
		TYPE13 = 13,
		TYPE14 = 14,
		TYPE15 = 15,
		TYPE16 = 16,
		TYPE17 = 17,
		TYPE99 = 99
	};

	enum RecordVariant {
		TYPE1_A,
		TYPE1_B,
		TYPE2_A,
		TYPE2_B,
		TYPE3_A,
		TYPE4_A,
		TYPE5_A,
		TYPE6_A,
		TYPE7_A,
		TYPE8_A,
		TYPE8_B,
		TYPE9_A,
		TYPE9_B,
		TYPE9_C,
		TYPE9_D,
		TYPE10_A,
		TYPE10_B,
		TYPE13_A,
		TYPE13_B,
		TYPE13_C,
		TYPE14_A,
		TYPE14_B,
		TYPE15_A,
		TYPE16_A,
		TYPE17_A,
		TYPE99_A,
		TYPE_UNKNOWN
	};

	RecordType findRecordType(string const& value);
	RecordType findRecordType(int value);
	auto_ptr<list<RecordVariant> > getRecordTypeVariants(RecordType recordType, bool ebts);
}

#endif
