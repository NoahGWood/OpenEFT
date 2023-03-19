#include "RecordType.hxx"
#include "utils.hxx"

#include <sstream>

namespace convert {
	using namespace std;

	RecordType findRecordType(string const& value) {
		return findRecordType(stringToInt(value));
	}

	RecordType findRecordType(int value) {
		switch(value) {
		case 1:
			return TYPE1;
		case 2:
			return TYPE2;
		case 3:
			return TYPE3;
		case 4:
			return TYPE4;
		case 5:
			return TYPE5;
		case 6:
			return TYPE6;
		case 7:
			return TYPE7;
		case 8:
			return TYPE8;
		case 9:
			return TYPE9;
		case 10:
			return TYPE10;
		case 13:
			return TYPE13;
		case 14:
			return TYPE14;
		case 15:
			return TYPE15;
		case 16:
			return TYPE16;
		case 17:
			return TYPE17;
		case 99:
			return TYPE99;
		}
	}

	auto_ptr<list<RecordVariant> > getRecordTypeVariants(RecordType recordType, bool ebts) {
		auto_ptr<list<RecordVariant> > recordTypeVariants(new list<RecordVariant>);
		switch(recordType) {
		case TYPE1:
			if(ebts) {
				recordTypeVariants->push_back(TYPE1_B);
			} else {
				recordTypeVariants->push_back(TYPE1_A);
			}
			break;
		case TYPE2:
			if(ebts) {
				recordTypeVariants->push_back(TYPE2_B);
			} else {
				recordTypeVariants->push_back(TYPE2_A);
			}
			break;
		case TYPE3:
			recordTypeVariants->push_back(TYPE3_A);
			break;
		case TYPE4:
			recordTypeVariants->push_back(TYPE4_A);
			break;
		case TYPE5:
			recordTypeVariants->push_back(TYPE5_A);
			break;
		case TYPE6:
			recordTypeVariants->push_back(TYPE6_A);
			break;
		case TYPE7:
			recordTypeVariants->push_back(TYPE7_A);
			break;
		case TYPE8:
			recordTypeVariants->push_back(TYPE8_A);
			recordTypeVariants->push_back(TYPE8_B);
			break;
		case TYPE9:
			recordTypeVariants->push_back(TYPE9_A);
			recordTypeVariants->push_back(TYPE9_B);
			recordTypeVariants->push_back(TYPE9_C);
			recordTypeVariants->push_back(TYPE9_D);
			break;
		case TYPE10:
			recordTypeVariants->push_back(TYPE10_A);
			recordTypeVariants->push_back(TYPE10_B);
			break;
		case TYPE13:
			recordTypeVariants->push_back(TYPE13_A);
			recordTypeVariants->push_back(TYPE13_B);
			recordTypeVariants->push_back(TYPE13_C);
			break;
		case TYPE14:
			recordTypeVariants->push_back(TYPE14_A);
			recordTypeVariants->push_back(TYPE14_B);
			break;
		case TYPE15:
			recordTypeVariants->push_back(TYPE15_A);
			break;
		case TYPE16:
			recordTypeVariants->push_back(TYPE16_A);
			break;
		case TYPE17:
			recordTypeVariants->push_back(TYPE17_A);
			break;
		case TYPE99:
			recordTypeVariants->push_back(TYPE99_A);
			break;
		}
		return recordTypeVariants;
	}
}
