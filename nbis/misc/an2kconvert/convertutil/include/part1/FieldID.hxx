#ifndef FIELDID_HXX
#define FIELDID_HXX

#include "RecordType.hxx"

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

namespace convert {
	namespace part1 {
		using namespace std;

		class FieldID {
		public:
			FieldID(RecordType recordType, int fieldNumber);
			RecordType getRecordType() const;
			int getFieldNumber() const;
			string toString() const;
			string toBytesForFile() const;
			bool operator==(FieldID const& fieldId) const;
			bool operator!=(FieldID const& fieldId) const;
			bool operator>(FieldID const& fieldId) const;
			bool operator<(FieldID const& fieldId) const;

			static const FieldID MISSING_FIELD;
			//Type 1 Fields
			static const FieldID TYPE_1_LEN;
			static const FieldID TYPE_1_VER;
			static const FieldID TYPE_1_CNT;
			static const FieldID TYPE_1_TOT;
			static const FieldID TYPE_1_DAT;
			static const FieldID TYPE_1_PRY;
			static const FieldID TYPE_1_DAI;
			static const FieldID TYPE_1_ORI;
			static const FieldID TYPE_1_TCN;
			static const FieldID TYPE_1_TCR;
			static const FieldID TYPE_1_NSR;
			static const FieldID TYPE_1_NTR;
			static const FieldID TYPE_1_DOM;
			static const FieldID TYPE_1_GMT;
			static const FieldID TYPE_1_DCS;
			//Type 2 Fields
			static const FieldID TYPE_2_LEN;
			static const FieldID TYPE_2_IDC;
			static const FieldID TYPE_2_FFN;
			static const FieldID TYPE_2_QDD;
			static const FieldID TYPE_2_SCO;
			static const FieldID TYPE_2_CIN;
			static const FieldID TYPE_2_CIX;
			static const FieldID TYPE_2_LCN;
			static const FieldID TYPE_2_LCX;
			static const FieldID TYPE_2_FBI;
			//Type 3 Fields
			static const FieldID TYPE_3_LEN;
			static const FieldID TYPE_3_IDC;
			static const FieldID TYPE_3_IMP;
			static const FieldID TYPE_3_FGP;
			static const FieldID TYPE_3_ISR;
			static const FieldID TYPE_3_HLL;
			static const FieldID TYPE_3_VLL;
			static const FieldID TYPE_3_CA;
			static const FieldID TYPE_3_DATA;
			//Type 4 Fields
			static const FieldID TYPE_4_LEN;
			static const FieldID TYPE_4_IDC;
			static const FieldID TYPE_4_IMP;
			static const FieldID TYPE_4_FGP;
			static const FieldID TYPE_4_ISR;
			static const FieldID TYPE_4_HLL;
			static const FieldID TYPE_4_VLL;
			static const FieldID TYPE_4_CA;
			static const FieldID TYPE_4_DATA;
			//Type 5 Fields
			static const FieldID TYPE_5_LEN;
			static const FieldID TYPE_5_IDC;
			static const FieldID TYPE_5_IMP;
			static const FieldID TYPE_5_FGP;
			static const FieldID TYPE_5_ISR;
			static const FieldID TYPE_5_HLL;
			static const FieldID TYPE_5_VLL;
			static const FieldID TYPE_5_CA;
			static const FieldID TYPE_5_DATA;
			//Type 6 Fields
			static const FieldID TYPE_6_LEN;
			static const FieldID TYPE_6_IDC;
			static const FieldID TYPE_6_IMP;
			static const FieldID TYPE_6_FGP;
			static const FieldID TYPE_6_ISR;
			static const FieldID TYPE_6_HLL;
			static const FieldID TYPE_6_VLL;
			static const FieldID TYPE_6_CA;
			static const FieldID TYPE_6_DATA;
			//Type 7 Fields
			static const FieldID TYPE_7_LEN;
			static const FieldID TYPE_7_IDC;
			//Type 8 Fields
			static const FieldID TYPE_8_LEN;
			static const FieldID TYPE_8_IDC;
			static const FieldID TYPE_8_SIG;
			static const FieldID TYPE_8_SRT;
			static const FieldID TYPE_8_ISR;
			static const FieldID TYPE_8_HLL;
			static const FieldID TYPE_8_VLL;
			static const FieldID TYPE_8_DATA;
			//Type 9 Fields
			static const FieldID TYPE_9_LEN;
			static const FieldID TYPE_9_IDC;
			static const FieldID TYPE_9_IMP;
			static const FieldID TYPE_9_FMT;
			static const FieldID TYPE_9_OFR;
			static const FieldID TYPE_9_FGP;
			static const FieldID TYPE_9_FPC;
			static const FieldID TYPE_9_CRP;
			static const FieldID TYPE_9_DLT;
			static const FieldID TYPE_9_MIN;
			static const FieldID TYPE_9_RDG;
			static const FieldID TYPE_9_MRC;
			static const FieldID TYPE_9_126;
			static const FieldID TYPE_9_127;
			static const FieldID TYPE_9_128;
			static const FieldID TYPE_9_129;
			static const FieldID TYPE_9_130;
			static const FieldID TYPE_9_131;
			static const FieldID TYPE_9_132;
			static const FieldID TYPE_9_133;
			static const FieldID TYPE_9_134;
			static const FieldID TYPE_9_135;
			static const FieldID TYPE_9_136;
			static const FieldID TYPE_9_137;
			static const FieldID TYPE_9_138;
			static const FieldID TYPE_9_139;
			static const FieldID TYPE_9_140;
			//Type 10 Fields
			static const FieldID TYPE_10_LEN;
			static const FieldID TYPE_10_IDC;
			static const FieldID TYPE_10_IMT;
			static const FieldID TYPE_10_SRC;
			static const FieldID TYPE_10_PHD;
			static const FieldID TYPE_10_HLL;
			static const FieldID TYPE_10_VLL;
			static const FieldID TYPE_10_SLC;
			static const FieldID TYPE_10_HPS;
			static const FieldID TYPE_10_VPS;
			static const FieldID TYPE_10_CGA;
			static const FieldID TYPE_10_CSP;
			static const FieldID TYPE_10_SAP;
			static const FieldID TYPE_10_SHPS;
			static const FieldID TYPE_10_SVPS;
			static const FieldID TYPE_10_POS;
			static const FieldID TYPE_10_POA;
			static const FieldID TYPE_10_PXS;
			static const FieldID TYPE_10_PAS;
			static const FieldID TYPE_10_SQS;
			static const FieldID TYPE_10_SPA;
			static const FieldID TYPE_10_SXS;
			static const FieldID TYPE_10_SEC;
			static const FieldID TYPE_10_SHC;
			static const FieldID TYPE_10_SFP;
			static const FieldID TYPE_10_DMM;
			static const FieldID TYPE_10_SMT;
			static const FieldID TYPE_10_SMS;
			static const FieldID TYPE_10_SMD;
			static const FieldID TYPE_10_COL;
			static const FieldID TYPE_10_DATA;
			//Type 13 Fields
			static const FieldID TYPE_13_LEN;
			static const FieldID TYPE_13_IDC;
			static const FieldID TYPE_13_IMP;
			static const FieldID TYPE_13_SRC;
			static const FieldID TYPE_13_LCD;
			static const FieldID TYPE_13_HLL;
			static const FieldID TYPE_13_VLL;
			static const FieldID TYPE_13_SLC;
			static const FieldID TYPE_13_HPS;
			static const FieldID TYPE_13_VPS;
			static const FieldID TYPE_13_CGA;
			static const FieldID TYPE_13_BPX;
			static const FieldID TYPE_13_FGP;
			static const FieldID TYPE_13_SPD;
			static const FieldID TYPE_13_PPC;
			static const FieldID TYPE_13_SHPS;
			static const FieldID TYPE_13_SVPS;
			static const FieldID TYPE_13_COM;
			static const FieldID TYPE_13_LQM;
			static const FieldID TYPE_13_DATA;
			//Type 14 Fields
			static const FieldID TYPE_14_LEN;
			static const FieldID TYPE_14_IDC;
			static const FieldID TYPE_14_IMP;
			static const FieldID TYPE_14_SRC;
			static const FieldID TYPE_14_FCD;
			static const FieldID TYPE_14_HLL;
			static const FieldID TYPE_14_VLL;
			static const FieldID TYPE_14_SLC;
			static const FieldID TYPE_14_HPS;
			static const FieldID TYPE_14_VPS;
			static const FieldID TYPE_14_CGA;
			static const FieldID TYPE_14_BPX;
			static const FieldID TYPE_14_FGP;
			static const FieldID TYPE_14_PPD;
			static const FieldID TYPE_14_PPC;
			static const FieldID TYPE_14_SHPS;
			static const FieldID TYPE_14_SVPS;
			static const FieldID TYPE_14_AMP;
			static const FieldID TYPE_14_COM;
			static const FieldID TYPE_14_SEG;
			static const FieldID TYPE_14_NQM;
			static const FieldID TYPE_14_SQM;
			static const FieldID TYPE_14_FQM;
			static const FieldID TYPE_14_ASEG;
			static const FieldID TYPE_14_DMM;
			static const FieldID TYPE_14_DATA;
			//Type 15 Fields
			static const FieldID TYPE_15_LEN;
			static const FieldID TYPE_15_IDC;
			static const FieldID TYPE_15_IMP;
			static const FieldID TYPE_15_SRC;
			static const FieldID TYPE_15_PCD;
			static const FieldID TYPE_15_HLL;
			static const FieldID TYPE_15_VLL;
			static const FieldID TYPE_15_SLC;
			static const FieldID TYPE_15_HPS;
			static const FieldID TYPE_15_VPS;
			static const FieldID TYPE_15_CGA;
			static const FieldID TYPE_15_BPX;
			static const FieldID TYPE_15_PLP;
			static const FieldID TYPE_15_SHPS;
			static const FieldID TYPE_15_SVPS;
			static const FieldID TYPE_15_COM;
			static const FieldID TYPE_15_PQM;
			static const FieldID TYPE_15_DMM;
			static const FieldID TYPE_15_DATA;
			//Type 16 Fields
			static const FieldID TYPE_16_LEN;
			static const FieldID TYPE_16_IDC;
			static const FieldID TYPE_16_UDI;
			static const FieldID TYPE_16_SRC;
			static const FieldID TYPE_16_UTD;
			static const FieldID TYPE_16_HLL;
			static const FieldID TYPE_16_VLL;
			static const FieldID TYPE_16_SLC;
			static const FieldID TYPE_16_HPS;
			static const FieldID TYPE_16_VPS;
			static const FieldID TYPE_16_CGA;
			static const FieldID TYPE_16_BPX;
			static const FieldID TYPE_16_CSP;
			static const FieldID TYPE_16_SHPS;
			static const FieldID TYPE_16_SVPS;
			static const FieldID TYPE_16_COM;
			static const FieldID TYPE_16_UQS;
			static const FieldID TYPE_16_DMM;
			static const FieldID TYPE_16_DATA;
			//Type 17 Fields
			static const FieldID TYPE_17_LEN;
			static const FieldID TYPE_17_IDC;
			static const FieldID TYPE_17_FID;
			static const FieldID TYPE_17_SRC;
			static const FieldID TYPE_17_ICD;
			static const FieldID TYPE_17_HLL;
			static const FieldID TYPE_17_VLL;
			static const FieldID TYPE_17_SLC;
			static const FieldID TYPE_17_HPS;
			static const FieldID TYPE_17_VPS;
			static const FieldID TYPE_17_CGA;
			static const FieldID TYPE_17_BPX;
			static const FieldID TYPE_17_CSP;
			static const FieldID TYPE_17_RAE;
			static const FieldID TYPE_17_RAU;
			static const FieldID TYPE_17_IPC;
			static const FieldID TYPE_17_DUI;
			static const FieldID TYPE_17_GUI;
			static const FieldID TYPE_17_MMS;
			static const FieldID TYPE_17_ECL;
			static const FieldID TYPE_17_COM;
			static const FieldID TYPE_17_SHPS;
			static const FieldID TYPE_17_SVPS;
			static const FieldID TYPE_17_IQS;
			static const FieldID TYPE_17_ALS;
			static const FieldID TYPE_17_IRD;
			static const FieldID TYPE_17_DMM;
			static const FieldID TYPE_17_DATA;
			//Type 99 Fields
			static const FieldID TYPE_99_LEN;
			static const FieldID TYPE_99_IDC;
			static const FieldID TYPE_99_SRC;
			static const FieldID TYPE_99_BCD;
			static const FieldID TYPE_99_HDV;
			static const FieldID TYPE_99_BTY;
			static const FieldID TYPE_99_BDQ;
			static const FieldID TYPE_99_BFO;
			static const FieldID TYPE_99_BFT;
			static const FieldID TYPE_99_BDB;

		private:
			RecordType recordType;
			int fieldNumber;
		};
	}
}

#endif
