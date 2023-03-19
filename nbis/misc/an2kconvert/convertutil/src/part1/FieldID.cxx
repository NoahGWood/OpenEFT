#include "part1/FieldID.hxx"

namespace convert {
	namespace part1 {
		using namespace std;

		/**
		 * Constructs a new FieldID from recordType and fieldNumber.
		 *
		 * recordType: the RecordType of the new FieldID.
		 * fieldNumber: the field number of the new FieldID.
		 */
		FieldID::FieldID(RecordType recordType, int fieldNumber)
		: recordType(recordType), fieldNumber(fieldNumber) {}


		/**
		 * Returns the RecordType of the FieldID.
		 */
		RecordType FieldID::getRecordType() const {
			return recordType;
		}


		/**
		 * Returns the field number of the FieldID.
		 */
		int FieldID::getFieldNumber() const {
			return fieldNumber;
		}


		/**
		 * Returns a string representation of the FieldID for Part 1 tagged fields.
		 */
		string FieldID::toString() const {
			stringstream ss;
			ss << recordType << "." << setfill('0') << setw(3) << fieldNumber;
			return ss.str();
		}


		/**
		 * Returns a string representation of the FieldID for Part 1 tagged fields.
		 */
		string FieldID::toBytesForFile() const {
			stringstream ss;
			ss << toString() << ":";
			return ss.str();
		}


		/**
		 * Equality comparison operator.  Returns true if the RecordType and field
		 * numbers are the same.
		 *
		 * fieldId: FieldID to compare.
		 */
		bool FieldID::operator==(FieldID const& fieldId) const {
			return this->getRecordType() == fieldId.getRecordType() && this->fieldNumber == fieldId.getFieldNumber();
		}


		/**
		 * Inequality comparison operator.  Returns the opposite of the equality comparison
		 * operator.
		 *
		 * fieldId: FieldID to compare.
		 */
		bool FieldID::operator!=(FieldID const& fieldId) const {
			return !(*this == fieldId);
		}


		/**
		 * Greater than comparison operator.  Returns true if the value of field number
		 * is greater than the value of field number in fieldId.  It is an error if the
		 * recordTypes aren't the same.
		 *
		 * fieldId: FieldID to compare.
		 */
		bool FieldID::operator>(FieldID const& fieldId) const {
			if(this->recordType != fieldId.getRecordType()) {
				throw logic_error("FieldID.operator>(): RecordTypes don't match");
			}
			return this->fieldNumber > fieldId.getFieldNumber();
		}


		/**
		 * Less than comparison operator.  Returns true if the value of field number
		 * is less than the value of field number in fieldId.  It is an error if the
		 * recordTypes aren't the same.
		 *
		 * fieldId: FieldID to compare.
		 */
		bool FieldID::operator<(FieldID const& fieldId) const {
			if(this->recordType != fieldId.getRecordType()) {
				throw logic_error("FieldID.operator>(): RecordTypes don't match");
			}
			return this->fieldNumber < fieldId.getFieldNumber();
		}


		const FieldID FieldID::MISSING_FIELD(TYPE1, -1);
		//Type 1 Fields
		const FieldID FieldID::TYPE_1_LEN(TYPE1, 1);
		const FieldID FieldID::TYPE_1_VER(TYPE1, 2);
		const FieldID FieldID::TYPE_1_CNT(TYPE1, 3);
		const FieldID FieldID::TYPE_1_TOT(TYPE1, 4);
		const FieldID FieldID::TYPE_1_DAT(TYPE1, 5);
		const FieldID FieldID::TYPE_1_PRY(TYPE1, 6);
		const FieldID FieldID::TYPE_1_DAI(TYPE1, 7);
		const FieldID FieldID::TYPE_1_ORI(TYPE1, 8);
		const FieldID FieldID::TYPE_1_TCN(TYPE1, 9);
		const FieldID FieldID::TYPE_1_TCR(TYPE1, 10);
		const FieldID FieldID::TYPE_1_NSR(TYPE1, 11);
		const FieldID FieldID::TYPE_1_NTR(TYPE1, 12);
		const FieldID FieldID::TYPE_1_DOM(TYPE1, 13);
		const FieldID FieldID::TYPE_1_GMT(TYPE1, 14);
		const FieldID FieldID::TYPE_1_DCS(TYPE1, 15);
		//Type 2 Fields
		const FieldID FieldID::TYPE_2_LEN(TYPE2, 1);
		const FieldID FieldID::TYPE_2_IDC(TYPE2, 2);
		const FieldID FieldID::TYPE_2_FFN(TYPE2, 3);
		const FieldID FieldID::TYPE_2_QDD(TYPE2, 4);
		const FieldID FieldID::TYPE_2_SCO(TYPE2, 7);
		const FieldID FieldID::TYPE_2_CIN(TYPE2, 10);
		const FieldID FieldID::TYPE_2_CIX(TYPE2, 11);
		const FieldID FieldID::TYPE_2_LCN(TYPE2, 12);
		const FieldID FieldID::TYPE_2_LCX(TYPE2, 13);
		const FieldID FieldID::TYPE_2_FBI(TYPE2, 14);
		//Type 3 Fields
		const FieldID FieldID::TYPE_3_LEN(TYPE3, 1);
		const FieldID FieldID::TYPE_3_IDC(TYPE3, 2);
		const FieldID FieldID::TYPE_3_IMP(TYPE3, 3);
		const FieldID FieldID::TYPE_3_FGP(TYPE3, 4);
		const FieldID FieldID::TYPE_3_ISR(TYPE3, 5);
		const FieldID FieldID::TYPE_3_HLL(TYPE3, 6);
		const FieldID FieldID::TYPE_3_VLL(TYPE3, 7);
		const FieldID FieldID::TYPE_3_CA(TYPE3, 8);
		const FieldID FieldID::TYPE_3_DATA(TYPE3, 9);
		//Type 4 Fields
		const FieldID FieldID::TYPE_4_LEN(TYPE4, 1);
		const FieldID FieldID::TYPE_4_IDC(TYPE4, 2);
		const FieldID FieldID::TYPE_4_IMP(TYPE4, 3);
		const FieldID FieldID::TYPE_4_FGP(TYPE4, 4);
		const FieldID FieldID::TYPE_4_ISR(TYPE4, 5);
		const FieldID FieldID::TYPE_4_HLL(TYPE4, 6);
		const FieldID FieldID::TYPE_4_VLL(TYPE4, 7);
		const FieldID FieldID::TYPE_4_CA(TYPE4, 8);
		const FieldID FieldID::TYPE_4_DATA(TYPE4, 9);
		//Type 5 Fields
		const FieldID FieldID::TYPE_5_LEN(TYPE5, 1);
		const FieldID FieldID::TYPE_5_IDC(TYPE5, 2);
		const FieldID FieldID::TYPE_5_IMP(TYPE5, 3);
		const FieldID FieldID::TYPE_5_FGP(TYPE5, 4);
		const FieldID FieldID::TYPE_5_ISR(TYPE5, 5);
		const FieldID FieldID::TYPE_5_HLL(TYPE5, 6);
		const FieldID FieldID::TYPE_5_VLL(TYPE5, 7);
		const FieldID FieldID::TYPE_5_CA(TYPE5, 8);
		const FieldID FieldID::TYPE_5_DATA(TYPE5, 9);
		//Type 6 Fields
		const FieldID FieldID::TYPE_6_LEN(TYPE6, 1);
		const FieldID FieldID::TYPE_6_IDC(TYPE6, 2);
		const FieldID FieldID::TYPE_6_IMP(TYPE6, 3);
		const FieldID FieldID::TYPE_6_FGP(TYPE6, 4);
		const FieldID FieldID::TYPE_6_ISR(TYPE6, 5);
		const FieldID FieldID::TYPE_6_HLL(TYPE6, 6);
		const FieldID FieldID::TYPE_6_VLL(TYPE6, 7);
		const FieldID FieldID::TYPE_6_CA(TYPE6, 8);
		const FieldID FieldID::TYPE_6_DATA(TYPE6, 9);
		//Type 7 Fields
		const FieldID FieldID::TYPE_7_LEN(TYPE7, 1);
		const FieldID FieldID::TYPE_7_IDC(TYPE7, 2);
		//Type 8 Fields
		const FieldID FieldID::TYPE_8_LEN(TYPE8, 1);
		const FieldID FieldID::TYPE_8_IDC(TYPE8, 2);
		const FieldID FieldID::TYPE_8_SIG(TYPE8, 3);
		const FieldID FieldID::TYPE_8_SRT(TYPE8, 4);
		const FieldID FieldID::TYPE_8_ISR(TYPE8, 5);
		const FieldID FieldID::TYPE_8_HLL(TYPE8, 6);
		const FieldID FieldID::TYPE_8_VLL(TYPE8, 7);
		const FieldID FieldID::TYPE_8_DATA(TYPE8, 8);
		//Type 9 Fields
		const FieldID FieldID::TYPE_9_LEN(TYPE9, 1);
		const FieldID FieldID::TYPE_9_IDC(TYPE9, 2);
		const FieldID FieldID::TYPE_9_IMP(TYPE9, 3);
		const FieldID FieldID::TYPE_9_FMT(TYPE9, 4);
		const FieldID FieldID::TYPE_9_OFR(TYPE9, 5);
		const FieldID FieldID::TYPE_9_FGP(TYPE9, 6);
		const FieldID FieldID::TYPE_9_FPC(TYPE9, 7);
		const FieldID FieldID::TYPE_9_CRP(TYPE9, 8);
		const FieldID FieldID::TYPE_9_DLT(TYPE9, 9);
		const FieldID FieldID::TYPE_9_MIN(TYPE9, 10);
		const FieldID FieldID::TYPE_9_RDG(TYPE9, 11);
		const FieldID FieldID::TYPE_9_MRC(TYPE9, 12);
		const FieldID FieldID::TYPE_9_126(TYPE9, 126);
		const FieldID FieldID::TYPE_9_127(TYPE9, 127);
		const FieldID FieldID::TYPE_9_128(TYPE9, 128);
		const FieldID FieldID::TYPE_9_129(TYPE9, 129);
		const FieldID FieldID::TYPE_9_130(TYPE9, 130);
		const FieldID FieldID::TYPE_9_131(TYPE9, 131);
		const FieldID FieldID::TYPE_9_132(TYPE9, 132);
		const FieldID FieldID::TYPE_9_133(TYPE9, 133);
		const FieldID FieldID::TYPE_9_134(TYPE9, 134);
		const FieldID FieldID::TYPE_9_135(TYPE9, 135);
		const FieldID FieldID::TYPE_9_136(TYPE9, 136);
		const FieldID FieldID::TYPE_9_137(TYPE9, 137);
		const FieldID FieldID::TYPE_9_138(TYPE9, 138);
		const FieldID FieldID::TYPE_9_139(TYPE9, 139);
		const FieldID FieldID::TYPE_9_140(TYPE9, 140);
		//Type 10 Fields
		const FieldID FieldID::TYPE_10_LEN(TYPE10, 1);
		const FieldID FieldID::TYPE_10_IDC(TYPE10, 2);
		const FieldID FieldID::TYPE_10_IMT(TYPE10, 3);
		const FieldID FieldID::TYPE_10_SRC(TYPE10, 4);
		const FieldID FieldID::TYPE_10_PHD(TYPE10, 5);
		const FieldID FieldID::TYPE_10_HLL(TYPE10, 6);
		const FieldID FieldID::TYPE_10_VLL(TYPE10, 7);
		const FieldID FieldID::TYPE_10_SLC(TYPE10, 8);
		const FieldID FieldID::TYPE_10_HPS(TYPE10, 9);
		const FieldID FieldID::TYPE_10_VPS(TYPE10, 10);
		const FieldID FieldID::TYPE_10_CGA(TYPE10, 11);
		const FieldID FieldID::TYPE_10_CSP(TYPE10, 12);
		const FieldID FieldID::TYPE_10_SAP(TYPE10, 13);
		const FieldID FieldID::TYPE_10_SHPS(TYPE10, 16);
		const FieldID FieldID::TYPE_10_SVPS(TYPE10, 17);
		const FieldID FieldID::TYPE_10_POS(TYPE10, 20);
		const FieldID FieldID::TYPE_10_POA(TYPE10, 21);
		const FieldID FieldID::TYPE_10_PXS(TYPE10, 22);
		const FieldID FieldID::TYPE_10_PAS(TYPE10, 23);
		const FieldID FieldID::TYPE_10_SQS(TYPE10, 24);
		const FieldID FieldID::TYPE_10_SPA(TYPE10, 25);
		const FieldID FieldID::TYPE_10_SXS(TYPE10, 26);
		const FieldID FieldID::TYPE_10_SEC(TYPE10, 27);
		const FieldID FieldID::TYPE_10_SHC(TYPE10, 28);
		const FieldID FieldID::TYPE_10_SFP(TYPE10, 29);
		const FieldID FieldID::TYPE_10_DMM(TYPE10, 30);
		const FieldID FieldID::TYPE_10_SMT(TYPE10, 40);
		const FieldID FieldID::TYPE_10_SMS(TYPE10, 41);
		const FieldID FieldID::TYPE_10_SMD(TYPE10, 42);
		const FieldID FieldID::TYPE_10_COL(TYPE10, 43);
		const FieldID FieldID::TYPE_10_DATA(TYPE10, 999);
		//Type 13 Fields
		const FieldID FieldID::TYPE_13_LEN(TYPE13, 1);
		const FieldID FieldID::TYPE_13_IDC(TYPE13, 2);
		const FieldID FieldID::TYPE_13_IMP(TYPE13, 3);
		const FieldID FieldID::TYPE_13_SRC(TYPE13, 4);
		const FieldID FieldID::TYPE_13_LCD(TYPE13, 5);
		const FieldID FieldID::TYPE_13_HLL(TYPE13, 6);
		const FieldID FieldID::TYPE_13_VLL(TYPE13, 7);
		const FieldID FieldID::TYPE_13_SLC(TYPE13, 8);
		const FieldID FieldID::TYPE_13_HPS(TYPE13, 9);
		const FieldID FieldID::TYPE_13_VPS(TYPE13, 10);
		const FieldID FieldID::TYPE_13_CGA(TYPE13, 11);
		const FieldID FieldID::TYPE_13_BPX(TYPE13, 12);
		const FieldID FieldID::TYPE_13_FGP(TYPE13, 13);
		const FieldID FieldID::TYPE_13_SPD(TYPE13, 14);
		const FieldID FieldID::TYPE_13_PPC(TYPE13, 15);
		const FieldID FieldID::TYPE_13_SHPS(TYPE13, 16);
		const FieldID FieldID::TYPE_13_SVPS(TYPE13, 17);
		const FieldID FieldID::TYPE_13_COM(TYPE13, 20);
		const FieldID FieldID::TYPE_13_LQM(TYPE13, 24);
		const FieldID FieldID::TYPE_13_DATA(TYPE13, 999);
		//Type 14 Fields
		const FieldID FieldID::TYPE_14_LEN(TYPE14, 1);
		const FieldID FieldID::TYPE_14_IDC(TYPE14, 2);
		const FieldID FieldID::TYPE_14_IMP(TYPE14, 3);
		const FieldID FieldID::TYPE_14_SRC(TYPE14, 4);
		const FieldID FieldID::TYPE_14_FCD(TYPE14, 5);
		const FieldID FieldID::TYPE_14_HLL(TYPE14, 6);
		const FieldID FieldID::TYPE_14_VLL(TYPE14, 7);
		const FieldID FieldID::TYPE_14_SLC(TYPE14, 8);
		const FieldID FieldID::TYPE_14_HPS(TYPE14, 9);
		const FieldID FieldID::TYPE_14_VPS(TYPE14, 10);
		const FieldID FieldID::TYPE_14_CGA(TYPE14, 11);
		const FieldID FieldID::TYPE_14_BPX(TYPE14, 12);
		const FieldID FieldID::TYPE_14_FGP(TYPE14, 13);
		const FieldID FieldID::TYPE_14_PPD(TYPE14, 14);
		const FieldID FieldID::TYPE_14_PPC(TYPE14, 15);
		const FieldID FieldID::TYPE_14_SHPS(TYPE14, 16);
		const FieldID FieldID::TYPE_14_SVPS(TYPE14, 17);
		const FieldID FieldID::TYPE_14_AMP(TYPE14, 18);
		const FieldID FieldID::TYPE_14_COM(TYPE14, 20);
		const FieldID FieldID::TYPE_14_SEG(TYPE14, 21);
		const FieldID FieldID::TYPE_14_NQM(TYPE14, 22);
		const FieldID FieldID::TYPE_14_SQM(TYPE14, 23);
		const FieldID FieldID::TYPE_14_FQM(TYPE14, 24);
		const FieldID FieldID::TYPE_14_ASEG(TYPE14, 25);
		const FieldID FieldID::TYPE_14_DMM(TYPE14, 30);
		const FieldID FieldID::TYPE_14_DATA(TYPE14, 999);
		//Type 15 Fields
		const FieldID FieldID::TYPE_15_LEN(TYPE15, 1);
		const FieldID FieldID::TYPE_15_IDC(TYPE15, 2);
		const FieldID FieldID::TYPE_15_IMP(TYPE15, 3);
		const FieldID FieldID::TYPE_15_SRC(TYPE15, 4);
		const FieldID FieldID::TYPE_15_PCD(TYPE15, 5);
		const FieldID FieldID::TYPE_15_HLL(TYPE15, 6);
		const FieldID FieldID::TYPE_15_VLL(TYPE15, 7);
		const FieldID FieldID::TYPE_15_SLC(TYPE15, 8);
		const FieldID FieldID::TYPE_15_HPS(TYPE15, 9);
		const FieldID FieldID::TYPE_15_VPS(TYPE15, 10);
		const FieldID FieldID::TYPE_15_CGA(TYPE15, 11);
		const FieldID FieldID::TYPE_15_BPX(TYPE15, 12);
		const FieldID FieldID::TYPE_15_PLP(TYPE15, 13);
		const FieldID FieldID::TYPE_15_SHPS(TYPE15, 16);
		const FieldID FieldID::TYPE_15_SVPS(TYPE15, 17);
		const FieldID FieldID::TYPE_15_COM(TYPE15, 20);
		const FieldID FieldID::TYPE_15_PQM(TYPE15, 24);
		const FieldID FieldID::TYPE_15_DMM(TYPE15, 30);
		const FieldID FieldID::TYPE_15_DATA(TYPE15, 999);
		//Type 16 Fields
		const FieldID FieldID::TYPE_16_LEN(TYPE16, 1);
		const FieldID FieldID::TYPE_16_IDC(TYPE16, 2);
		const FieldID FieldID::TYPE_16_UDI(TYPE16, 3);
		const FieldID FieldID::TYPE_16_SRC(TYPE16, 4);
		const FieldID FieldID::TYPE_16_UTD(TYPE16, 5);
		const FieldID FieldID::TYPE_16_HLL(TYPE16, 6);
		const FieldID FieldID::TYPE_16_VLL(TYPE16, 7);
		const FieldID FieldID::TYPE_16_SLC(TYPE16, 8);
		const FieldID FieldID::TYPE_16_HPS(TYPE16, 9);
		const FieldID FieldID::TYPE_16_VPS(TYPE16, 10);
		const FieldID FieldID::TYPE_16_CGA(TYPE16, 11);
		const FieldID FieldID::TYPE_16_BPX(TYPE16, 12);
		const FieldID FieldID::TYPE_16_CSP(TYPE16, 13);
		const FieldID FieldID::TYPE_16_SHPS(TYPE16, 16);
		const FieldID FieldID::TYPE_16_SVPS(TYPE16, 17);
		const FieldID FieldID::TYPE_16_COM(TYPE16, 20);
		const FieldID FieldID::TYPE_16_UQS(TYPE16, 24);
		const FieldID FieldID::TYPE_16_DMM(TYPE16, 30);
		const FieldID FieldID::TYPE_16_DATA(TYPE16, 999);
		//Type 17 Fields
		const FieldID FieldID::TYPE_17_LEN(TYPE17, 1);
		const FieldID FieldID::TYPE_17_IDC(TYPE17, 2);
		const FieldID FieldID::TYPE_17_FID(TYPE17, 3);
		const FieldID FieldID::TYPE_17_SRC(TYPE17, 4);
		const FieldID FieldID::TYPE_17_ICD(TYPE17, 5);
		const FieldID FieldID::TYPE_17_HLL(TYPE17, 6);
		const FieldID FieldID::TYPE_17_VLL(TYPE17, 7);
		const FieldID FieldID::TYPE_17_SLC(TYPE17, 8);
		const FieldID FieldID::TYPE_17_HPS(TYPE17, 9);
		const FieldID FieldID::TYPE_17_VPS(TYPE17, 10);
		const FieldID FieldID::TYPE_17_CGA(TYPE17, 11);
		const FieldID FieldID::TYPE_17_BPX(TYPE17, 12);
		const FieldID FieldID::TYPE_17_CSP(TYPE17, 13);
		const FieldID FieldID::TYPE_17_RAE(TYPE17, 14);
		const FieldID FieldID::TYPE_17_RAU(TYPE17, 15);
		const FieldID FieldID::TYPE_17_IPC(TYPE17, 16);
		const FieldID FieldID::TYPE_17_DUI(TYPE17, 17);
		const FieldID FieldID::TYPE_17_GUI(TYPE17, 18);
		const FieldID FieldID::TYPE_17_MMS(TYPE17, 19);
		const FieldID FieldID::TYPE_17_ECL(TYPE17, 20);
		const FieldID FieldID::TYPE_17_COM(TYPE17, 21);
		const FieldID FieldID::TYPE_17_SHPS(TYPE17, 22);
		const FieldID FieldID::TYPE_17_SVPS(TYPE17, 23);
		const FieldID FieldID::TYPE_17_IQS(TYPE17, 24);
		const FieldID FieldID::TYPE_17_ALS(TYPE17, 25);
		const FieldID FieldID::TYPE_17_IRD(TYPE17, 26);
		const FieldID FieldID::TYPE_17_DMM(TYPE17, 30);
		const FieldID FieldID::TYPE_17_DATA(TYPE17, 999);
		//Type 99 Fields
		const FieldID FieldID::TYPE_99_LEN(TYPE99, 1);
		const FieldID FieldID::TYPE_99_IDC(TYPE99, 2);
		const FieldID FieldID::TYPE_99_SRC(TYPE99, 4);
		const FieldID FieldID::TYPE_99_BCD(TYPE99, 5);
		const FieldID FieldID::TYPE_99_HDV(TYPE99, 100);
		const FieldID FieldID::TYPE_99_BTY(TYPE99, 101);
		const FieldID FieldID::TYPE_99_BDQ(TYPE99, 102);
		const FieldID FieldID::TYPE_99_BFO(TYPE99, 103);
		const FieldID FieldID::TYPE_99_BFT(TYPE99, 104);
		const FieldID FieldID::TYPE_99_BDB(TYPE99, 999);
	}
}
