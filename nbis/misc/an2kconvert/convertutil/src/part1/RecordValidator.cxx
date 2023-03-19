#include "part1/RecordValidator.hxx"
#include "utils.hxx"

#include <iostream>

namespace convert {
	namespace part1 {
		using namespace std;

		RecordValidator::~RecordValidator() {
			deleteContents<FieldValidator*>(fieldValidators);
		}

		RecordVariant RecordValidator::validateRecord(Record const& part1Record, ValidationLevel vl, bool ebts) {
			auto_ptr<list<RecordVariant> > recordVariants = getRecordTypeVariants(part1Record.getRecordType(), ebts);
			list<ValidationResult*> valResults;
			cout << "Processing Type " << part1Record.getRecordType() << " record......";
			for(list<RecordVariant>::const_iterator it = recordVariants->begin(); it != recordVariants->end(); it++) {
				RecordVariant recordVariant = *it;
				RecordValidator rv(part1Record.getRecordType(), recordVariant);
				valResults.push_back(rv.validate(part1Record, vl).release());
				if(!valResults.back()->hasError()) {
					if(valResults.back()->hasWarning()) {
						cout << "warning" << endl;
						valResults.back()->printWarnings(cout);
					} else {
						cout << "done" << endl;
					}
					deleteContents<ValidationResult*>(valResults);
					return recordVariant;
				}
			}

			stringstream ss;
			cout << "error" << endl;
			ss << "Type " << part1Record.getRecordType() << " record has the following errors:" << endl;
			int errorsPrinted = 0;
			for(list<ValidationResult*>::const_iterator it = valResults.begin(); it != valResults.end(); it++) {
				ValidationResult const& valResult = *(*it);
				valResult.printErrors(ss);
				errorsPrinted++;
				if(errorsPrinted != valResults.size()) {
					ss << "or" << endl;
				}
			}
			ss << endl;
			deleteContents<ValidationResult*>(valResults);
			throw ValidationError(ss.str());
		}

		RecordValidator::RecordValidator(RecordType recordType, RecordVariant recordVariant)
		: recordType(recordType), recordVariant(recordVariant) {
			switch(recordVariant) {
			case TYPE1_A:
			case TYPE1_B: configureType1Validator(); break;
			case TYPE2_A: configureType2AValidator(); break;
			case TYPE2_B: configureType2BValidator(); break;
			case TYPE3_A:
			case TYPE4_A:
			case TYPE5_A:
			case TYPE6_A: configureType3_6Validator(recordType); break;
			case TYPE7_A: configureType7Validator(); break;
			case TYPE8_A: configureType8AValidator(); break;
			case TYPE8_B: configureType8BValidator(); break;
			case TYPE9_A: configureType9AValidator(); break;
			case TYPE9_B: configureType9BValidator(); break;
			case TYPE9_C: configureType9CValidator(); break;
			case TYPE9_D: configureType9DValidator(); break;
			case TYPE10_A: configureType10AValidator(); break;
			case TYPE10_B: configureType10BValidator(); break;
			case TYPE13_A: configureType13AValidator(); break;
			case TYPE13_B: configureType13BValidator(); break;
			case TYPE13_C: configureType13CValidator(); break;
			case TYPE14_A: configureType14AValidator(); break;
			case TYPE14_B: configureType14BValidator(); break;
			case TYPE15_A: configureType15Validator(); break;
			case TYPE16_A: configureType16Validator(); break;
			case TYPE17_A: configureType17Validator(); break;
			case TYPE99_A: configureType99Validator(); break;
			}
		}

		void RecordValidator::addSimpleField(FieldID const& fieldId, FieldRequired fieldRequired, TextValidator* textValidator) {
			addField(new FieldValidator(fieldId, fieldRequired, SubfieldMin(1), SubfieldMax(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1), textValidator));
				finishSubfield();
			finishField();
		}

		void RecordValidator::addField(FieldValidator* validator) {
			if(currField.get() != NULL) {
				throw logic_error("Part1RecordValidator.addField(): can't add field before finishing current field");
			}
			currField = auto_ptr<FieldValidator>(validator);
		}

		void RecordValidator::finishField() {
			if(currField.get() == NULL) {
				throw logic_error("Part1RecordValidator.finishField(): can't finish field before adding field");
			}
			if(currSubfield.get() != NULL) {
				throw logic_error("Part1RecordValidator.finishField(): can't finish field before finishing current subfield");
			}
			fieldValidators.push_back(currField.release());
		}

		void RecordValidator::addSubfield(SubfieldValidator* validator) {
			if(currField.get() == NULL) {
				throw logic_error("Part1RecordValidator.addSubfield(): can't add subfield before adding field");
			}
			if(currSubfield.get() != NULL) {
				throw logic_error("Part1RecordValidator.addSubfield(): can't add subfield before finishing current subfield");
			}
			currSubfield = auto_ptr<SubfieldValidator>(validator);
		}

		void RecordValidator::finishSubfield() {
			if(currSubfield.get() == NULL) {
				throw logic_error("Part1RecordValidator.finishSufield(): can't finish subfield before adding subfield");
			}
			currField->addValidator(currSubfield);
		}

		void RecordValidator::addItem(ItemValidator* validator) {
			if(currSubfield.get() == NULL) {
				throw logic_error("Part1RecordValidator.addItem(): can't add item before adding subfield");
			}
			currSubfield->addValidator(auto_ptr<ItemValidator>(validator));
		}

		bool RecordValidator::hasFieldValidator(FieldID const& fieldId) const {
			for(list<FieldValidator*>::const_iterator it = fieldValidators.begin(); it != fieldValidators.end(); it++) {
				FieldValidator* val = *it;
				if(val->getFieldId() == fieldId) {
					return true;
				}
			}
			return false;
		}

		auto_ptr<ValidationResult> RecordValidator::validate(Record const& part1Record, ValidationLevel vl) {
			if(currField.get() != NULL || currSubfield.get() != NULL) {
				throw logic_error("Part1RecordValidator.validate(): didn't finish initialization");
			}

			auto_ptr<ValidationResult> valResult(new ValidationResult(part1Record.getRecordType(), recordVariant));
			for(list<FieldValidator*>::const_iterator it = fieldValidators.begin(); it != fieldValidators.end(); it++) {
				try {
					FieldValidator const& val = *(*it);
					Context context(val.getFieldId());
					if(!part1Record.hasField(val.getFieldId())) {
						Field field(FieldID::MISSING_FIELD);
						val.validateField(field, context, *valResult, vl);
					} else {
						Field const& field = part1Record.getField(val.getFieldId());
						val.validateField(field, context, *valResult, vl);
					}
				} catch(ValidationError&) {}
			}
			list<Field*> const& fields = part1Record.getFields();
			for(list<Field*>::const_iterator it = fields.begin(); it != fields.end(); it++) {
				Field const& field = *(*it);
				if(!hasFieldValidator(field.getFieldID())) {
					Context context(field.getFieldID());
					valResult->addWarning(auto_ptr<ValidationMessage>(new ValidationMessage(context, "Field will not be processed")));
				}
			}
			return valResult;
		}

		void RecordValidator::configureType1Validator() {
			addSimpleField(FieldID::TYPE_1_LEN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_1_VER, MANDATORY, new SingleValueValidator("0400", RelaxedWarning));

			addField(new FieldValidator(FieldID::TYPE_1_CNT, MANDATORY, SubfieldMin(2)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(2), ItemMax(2)));
					addItem(new ItemValidator(ItemCount(1), new NumericValueValidator(1)));
					addItem(new ItemValidator(ItemCount(1), new NNIntValidator));
				finishSubfield();
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(2), ItemMax(2)));
					addItem(new ItemValidator(ItemCount(2), new NNIntValidator));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_1_TOT, MANDATORY);
			addSimpleField(FieldID::TYPE_1_DAT, MANDATORY, new DateValidator);
			addSimpleField(FieldID::TYPE_1_PRY, OPTIONAL, new NumericRangeValidator(1, 9));
			addSimpleField(FieldID::TYPE_1_DAI, MANDATORY);
			addSimpleField(FieldID::TYPE_1_ORI, MANDATORY);
			addSimpleField(FieldID::TYPE_1_TCN, MANDATORY);
			addSimpleField(FieldID::TYPE_1_TCR, OPTIONAL);
			addSimpleField(FieldID::TYPE_1_NSR, MANDATORY, new ResolutionValidator);
			addSimpleField(FieldID::TYPE_1_NTR, MANDATORY, new ResolutionValidator);

			addField(new FieldValidator(FieldID::TYPE_1_DOM, OPTIONAL, SubfieldMin(1), SubfieldMax(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(1), ItemMax(2)));
					addItem(new ItemValidator(ItemCount(2)));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_1_GMT, OPTIONAL, new UTCDateValidator);

			addField(new FieldValidator(FieldID::TYPE_1_DCS, OPTIONAL, SubfieldMin(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(2), ItemMax(3)));
					addItem(new ItemValidator(ItemCount(3)));
				finishSubfield();
			finishField();
		}

		void RecordValidator::configureType2AValidator() {
			addSimpleField(FieldID::TYPE_2_LEN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_2_IDC, MANDATORY, new NNIntValidator);
		}

		void RecordValidator::configureType2BValidator() {
			addSimpleField(FieldID::TYPE_2_LEN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_2_IDC, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_2_FFN, OPTIONAL_WARNING, new NNIntValidator(10, 10));
			addSimpleField(FieldID::TYPE_2_QDD, OPTIONAL, new MultipleValueValidator("S|O|C"));

			addField(new FieldValidator(FieldID::TYPE_2_SCO, OPTIONAL, SubfieldMin(1), SubfieldMax(9)));
				addSubfield(new SubfieldValidator(SubfieldCount(9), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1), new TextLengthValidator(9, 19)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_2_CIN, OPTIONAL_WARNING, SubfieldMin(1), SubfieldMax(5, 200)));
				addSubfield(new SubfieldValidator(SubfieldCount(200), ItemMin(2), ItemMax(2)));
					addItem(new ItemValidator(ItemCount(2), new TextLengthValidator(1, 24)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_2_CIX, OPTIONAL_WARNING, SubfieldMin(1), SubfieldMax(5, 200)));
				addSubfield(new SubfieldValidator(SubfieldCount(200), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1), new NNIntValidator(2, 4)));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_2_LCN, OPTIONAL_WARNING, new Type2LCNValidator);
			addSimpleField(FieldID::TYPE_2_LCX, OPTIONAL_WARNING, new NNIntValidator(4, 4));

			addField(new FieldValidator(FieldID::TYPE_2_FBI, OPTIONAL, SubfieldMin(1), SubfieldMax(5, 1000)));
				addSubfield(new SubfieldValidator(SubfieldCount(1000), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1), new TextLengthValidator(9, 9)));
				finishSubfield();
			finishField();
		}

		void RecordValidator::configureType3_6Validator(RecordType recordType) {
			addSimpleField(FieldID(recordType, 1), MANDATORY, new U32Validator);
			addSimpleField(FieldID(recordType, 2), MANDATORY, new U8Validator);
			addSimpleField(FieldID(recordType, 3), MANDATORY, new Table11Validator);

			addField(new FieldValidator(FieldID(recordType, 4), MANDATORY, SubfieldMin(6), SubfieldMax(6)));
				addSubfield(new SubfieldValidator(SubfieldCount(6), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1), new Type3_6FGPValidator));
				finishSubfield();
			finishField();

			addSimpleField(FieldID(recordType, 5), MANDATORY, new NumericRangeValidator(0, 1));
			addSimpleField(FieldID(recordType, 6), MANDATORY, new U16Validator);
			addSimpleField(FieldID(recordType, 7), MANDATORY, new U16Validator);
			addSimpleField(FieldID(recordType, 8), MANDATORY, new U8Validator);
			addSimpleField(FieldID(recordType, 9), MANDATORY, new TextLengthValidator(1));
		}

		void RecordValidator::configureType7Validator() {
			addSimpleField(FieldID::TYPE_7_LEN, MANDATORY, new U32Validator);
			addSimpleField(FieldID::TYPE_7_IDC, MANDATORY, new U8Validator);
		}

		void RecordValidator::configureType8Validator() {
			addSimpleField(FieldID::TYPE_8_LEN, MANDATORY, new U32Validator);
			addSimpleField(FieldID::TYPE_8_IDC, MANDATORY, new U8Validator);
			addSimpleField(FieldID::TYPE_8_SIG, MANDATORY, new NumericRangeValidator(0, 1));
		}

		void RecordValidator::configureType8AValidator() {
			configureType8Validator();

			addSimpleField(FieldID::TYPE_8_SRT, MANDATORY, new NumericRangeValidator(0, 1));
			addSimpleField(FieldID::TYPE_8_ISR, MANDATORY, new NumericRangeValidator(0, 1));
			addSimpleField(FieldID::TYPE_8_HLL, MANDATORY, new U16Validator);
			addSimpleField(FieldID::TYPE_8_VLL, MANDATORY, new U16Validator);
			addSimpleField(FieldID::TYPE_8_DATA, MANDATORY, new TextLengthValidator(1));
		}

		void RecordValidator::configureType8BValidator() {
			configureType8Validator();

			addSimpleField(FieldID::TYPE_8_SRT, MANDATORY, new NumericValueValidator(2));
			addSimpleField(FieldID::TYPE_8_ISR, MANDATORY, new NumericValueValidator(0));
			addSimpleField(FieldID::TYPE_8_HLL, MANDATORY, new NumericValueValidator(0));
			addSimpleField(FieldID::TYPE_8_VLL, MANDATORY, new NumericValueValidator(0));

			addField(new FieldValidator(FieldID::TYPE_8_DATA, MANDATORY, SubfieldMin(2), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(3), ItemMax(3)));
					addItem(new ItemValidator(ItemCount(2), new U16Validator));
					addItem(new ItemValidator(ItemCount(1), new U8Validator));
				finishSubfield();
			finishField();
		}

		void RecordValidator::configureType9AValidator() {
			addSimpleField(FieldID::TYPE_9_LEN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_9_IDC, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_9_IMP, MANDATORY, new Table11Validator);
			addSimpleField(FieldID::TYPE_9_FMT, MANDATORY, new SingleValueValidator("S"));

			addField(new FieldValidator(FieldID::TYPE_9_OFR, OPTIONAL, SubfieldMin(1), SubfieldMax(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(2), ItemMax(3)));
					addItem(new ItemValidator(ItemCount(1)));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("A|U|E|M")));
					addItem(new ItemValidator(ItemCount(1), new TextLengthValidator(2, 2)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_9_FGP, MANDATORY, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1), new Table12Validator));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_9_FPC, MANDATORY, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(2), ItemMax(2)));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("T|U")));
					addItem(new ItemValidator(ItemCount(1)));
				finishSubfield();
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_9_CRP, OPTIONAL, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1), new NNIntValidator(8, 8)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_9_DLT, OPTIONAL, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1), new NNIntValidator(8, 8)));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_9_MIN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_9_RDG, MANDATORY, new MultipleValueValidator("0|1"));

			addField(new FieldValidator(FieldID::TYPE_9_MRC, OPTIONAL, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(2), ItemMax(INF)));
					addItem(new ItemValidator(ItemCount(1), new NNIntValidator));
					addItem(new ItemValidator(ItemCount(1), new NNIntValidator(11, 11)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 63)));
					addItem(new ItemValidator(ItemCount(1), new Table15Validator));
					addItem(new ItemValidator(ItemCount(INF), new Type9MRCValidator));
				finishSubfield();
			finishField();
		}

		void RecordValidator::configureType9BValidator() {
			addSimpleField(FieldID::TYPE_9_LEN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_9_IDC, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_9_IMP, MANDATORY, new Table11Validator);
			addSimpleField(FieldID::TYPE_9_FMT, MANDATORY, new SingleValueValidator("S"));

			addField(new FieldValidator(FieldID::TYPE_9_OFR, OPTIONAL, SubfieldMin(1), SubfieldMax(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(2), ItemMax(3)));
					addItem(new ItemValidator(ItemCount(1)));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("A|U|E|M")));
					addItem(new ItemValidator(ItemCount(1), new TextLengthValidator(2, 2)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_9_FGP, MANDATORY, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1), new Table35Validator));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_9_MIN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_9_RDG, MANDATORY, new MultipleValueValidator("0|1"));

			addField(new FieldValidator(FieldID::TYPE_9_MRC, OPTIONAL, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(2), ItemMax(INF)));
					addItem(new ItemValidator(ItemCount(1), new NNIntValidator));
					addItem(new ItemValidator(ItemCount(1), new NNIntValidator(13, 13)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 63)));
					addItem(new ItemValidator(ItemCount(1), new Table15Validator));
					addItem(new ItemValidator(ItemCount(INF), new Type9MRCValidator));
				finishSubfield();
			finishField();
		}

		void RecordValidator::configureType9CValidator() {
			addSimpleField(FieldID::TYPE_9_LEN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_9_IDC, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_9_IMP, MANDATORY, new Table11Validator);
			addSimpleField(FieldID::TYPE_9_FMT, MANDATORY, new SingleValueValidator("U"));
		}

		void RecordValidator::configureType9DValidator() {
			addSimpleField(FieldID::TYPE_9_LEN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_9_IDC, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_9_IMP, MANDATORY, new Table11Validator);
			addSimpleField(FieldID::TYPE_9_FMT, MANDATORY, new SingleValueValidator("U"));

			addField(new FieldValidator(FieldID::TYPE_9_126, MANDATORY, SubfieldMin(1), SubfieldMax(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(3), ItemMax(3)));
					addItem(new ItemValidator(ItemCount(1), new SingleValueValidator("27")));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("513|514")));
					addItem(new ItemValidator(ItemCount(1)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_9_127, MANDATORY, SubfieldMin(1), SubfieldMax(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(2), ItemMax(2)));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("AFFP|NONE")));
					addItem(new ItemValidator(ItemCount(1)));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_9_128, MANDATORY, new NumericRangeValidator(0, 65534));
			addSimpleField(FieldID::TYPE_9_129, MANDATORY, new NumericRangeValidator(0, 65534));
			addSimpleField(FieldID::TYPE_9_130, MANDATORY, new MultipleValueValidator("0|1|2"));
			addSimpleField(FieldID::TYPE_9_131, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_9_132, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_9_133, MANDATORY, new NumericRangeValidator(0, 15));
			addField(new FieldValidator(FieldID::TYPE_9_134, MANDATORY, SubfieldMin(1), SubfieldMax(1, INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_9_135, MANDATORY, SubfieldMin(1), SubfieldMax(1, INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(3), ItemMax(3)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 255)));
					addItem(new ItemValidator(ItemCount(1), new HexValidator(4, 4)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 65535)));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_9_136, MANDATORY, new NNIntValidator);

			addField(new FieldValidator(FieldID::TYPE_9_137, MANDATORY, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(6), ItemMax(6)));
					addItem(new ItemValidator(ItemCount(3), new NNIntValidator));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 179)));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("0|1|2")));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 100)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_9_138, OPTIONAL, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(3), ItemMax(3)));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("0|1|2")));
					addItem(new ItemValidator(ItemCount(2), new SingleValueValidator("0")));
				finishSubfield();
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(3), ItemMax(3)));
					addItem(new ItemValidator(ItemCount(3), new NNIntValidator));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_9_139, OPTIONAL, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(3), ItemMax(3)));
					addItem(new ItemValidator(ItemCount(2), new NNIntValidator));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 179)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_9_140, OPTIONAL, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(3), ItemMax(3)));
					addItem(new ItemValidator(ItemCount(2), new NNIntValidator));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 179)));
				finishSubfield();
			finishField();
		}

		void RecordValidator::configureType10AValidator() {
			addSimpleField(FieldID::TYPE_10_LEN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_10_IDC, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_10_IMT, MANDATORY, new MultipleValueValidator("SCAR|MARK|TATTOO"));
			addSimpleField(FieldID::TYPE_10_SRC, MANDATORY);
			addSimpleField(FieldID::TYPE_10_PHD, MANDATORY, new DateValidator);
			addSimpleField(FieldID::TYPE_10_HLL, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_10_VLL, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_10_SLC, MANDATORY, new MultipleValueValidator("0|1|2"));
			addSimpleField(FieldID::TYPE_10_HPS, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_10_VPS, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_10_CGA, MANDATORY, new Table1Validator);
			addSimpleField(FieldID::TYPE_10_CSP, MANDATORY, new Table3Validator);
			addSimpleField(FieldID::TYPE_10_SHPS, OPTIONAL, new NNIntValidator);
			addSimpleField(FieldID::TYPE_10_SVPS, OPTIONAL, new NNIntValidator);

			addField(new FieldValidator(FieldID::TYPE_10_SMT, MANDATORY, SubfieldMin(1), SubfieldMax(3, INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_10_SMS, OPTIONAL, SubfieldMin(1), SubfieldMax(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(2), ItemMax(2)));
					addItem(new ItemValidator(ItemCount(2), new NNIntValidator));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_10_SMD, OPTIONAL, SubfieldMin(1), SubfieldMax(9, INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(3), ItemMax(4)));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("SCAR|MARK|TATTOO|CHEMICAL|BRANDED|CUT")));
					addItem(new ItemValidator(ItemCount(1), new Table28Validator));
					addItem(new ItemValidator(ItemCount(1), new Table29Validator));
					addItem(new ItemValidator(ItemCount(1)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_10_COL, OPTIONAL, SubfieldMin(1), SubfieldMax(9, INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(1), ItemMax(INF)));
					addItem(new ItemValidator(ItemCount(INF), new Table30Validator));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_10_DATA, MANDATORY, new TextLengthValidator(1));
		}

		void RecordValidator::configureType10BValidator() {
			addSimpleField(FieldID::TYPE_10_LEN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_10_IDC, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_10_IMT, MANDATORY, new SingleValueValidator("FACE"));
			addSimpleField(FieldID::TYPE_10_SRC, MANDATORY);
			addSimpleField(FieldID::TYPE_10_PHD, MANDATORY, new DateValidator);
			addSimpleField(FieldID::TYPE_10_HLL, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_10_VLL, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_10_SLC, MANDATORY, new MultipleValueValidator("0|1|2"));
			addSimpleField(FieldID::TYPE_10_HPS, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_10_VPS, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_10_CGA, MANDATORY, new Table1Validator);
			addSimpleField(FieldID::TYPE_10_CSP, MANDATORY, new Table3Validator);
			addSimpleField(FieldID::TYPE_10_SAP, MANDATORY, new Table18Validator);
			addSimpleField(FieldID::TYPE_10_SHPS, OPTIONAL, new NNIntValidator);
			addSimpleField(FieldID::TYPE_10_SVPS, OPTIONAL, new NNIntValidator);
			addSimpleField(FieldID::TYPE_10_POS, OPTIONAL, new Table19Validator);
			addSimpleField(FieldID::TYPE_10_POA, OPTIONAL, new IntRangeValidator(-180, 180));

			addField(new FieldValidator(FieldID::TYPE_10_PXS, OPTIONAL, SubfieldMin(1), SubfieldMax(9, INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(1), ItemMax(2)));
					addItem(new ItemValidator(ItemCount(1), new Table20Validator));
					addItem(new ItemValidator(ItemCount(1)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_10_PAS, OPTIONAL, SubfieldMin(1), SubfieldMax(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(1), ItemMax(2)));
					addItem(new ItemValidator(ItemCount(1), new Table21Validator));
					addItem(new ItemValidator(ItemCount(1)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_10_SQS, OPTIONAL, SubfieldMin(1), SubfieldMax(1, 9)));
				addSubfield(new SubfieldValidator(SubfieldCount(9), ItemMin(3), ItemMax(3)));
				addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 255)));
				addItem(new ItemValidator(ItemCount(1), new HexValidator(4, 4)));
				addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 65535)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_10_SPA, OPTIONAL, SubfieldMin(1), SubfieldMax(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(6), ItemMax(6)));
					addItem(new ItemValidator(ItemCount(6), new NNIntValidator));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_10_SXS, OPTIONAL, SubfieldMin(1), SubfieldMax(50, INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1)));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_10_SEC, OPTIONAL, new Table23Validator);

			addField(new FieldValidator(FieldID::TYPE_10_SHC, OPTIONAL, SubfieldMin(1), SubfieldMax(2)));
				addSubfield(new SubfieldValidator(SubfieldCount(2), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1), new Table24Validator));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_10_SFP, OPTIONAL, SubfieldMin(1), SubfieldMax(88, INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(4), ItemMax(4)));
					addItem(new ItemValidator(ItemCount(1), new SingleValueValidator("1")));
					addItem(new ItemValidator(ItemCount(1), new FeaturePointCodeValidator));
					addItem(new ItemValidator(ItemCount(1), new NNIntValidator(1, 4)));
					addItem(new ItemValidator(ItemCount(1), new NNIntValidator(1, 4)));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_10_DMM, OPTIONAL, new Table27Validator);
			addSimpleField(FieldID::TYPE_10_DATA, MANDATORY, new TextLengthValidator(1));
		}

		void RecordValidator::configureType13Validator() {
			addSimpleField(FieldID::TYPE_13_LEN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_13_IDC, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_13_SRC, MANDATORY);
			addSimpleField(FieldID::TYPE_13_LCD, MANDATORY, new DateValidator);
			addSimpleField(FieldID::TYPE_13_HLL, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_13_VLL, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_13_SLC, MANDATORY, new MultipleValueValidator("0|1|2"));
			addSimpleField(FieldID::TYPE_13_HPS, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_13_VPS, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_13_CGA, MANDATORY, new Table1Validator);
			addSimpleField(FieldID::TYPE_13_BPX, MANDATORY, new NNIntValidator);

			addSimpleField(FieldID::TYPE_13_SHPS, OPTIONAL, new NNIntValidator);
			addSimpleField(FieldID::TYPE_13_SVPS, OPTIONAL, new NNIntValidator);
			addSimpleField(FieldID::TYPE_13_COM, OPTIONAL);

			addSimpleField(FieldID::TYPE_13_DATA, MANDATORY, new TextLengthValidator(1));
		}

		void RecordValidator::configureType13AValidator() {
			configureType13Validator();

			addSimpleField(FieldID::TYPE_13_IMP, MANDATORY, new Table11Validator);
			//addSimpleField(FieldID::TYPE_13_IMP, MANDATORY, new NumericRangeValidator(4, 7));

			addField(new FieldValidator(FieldID::TYPE_13_FGP, MANDATORY, SubfieldMin(1), SubfieldMax(6)));
				addSubfield(new SubfieldValidator(SubfieldCount(6), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 15)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_13_LQM, OPTIONAL, SubfieldMin(1), SubfieldMax(4)));
				addSubfield(new SubfieldValidator(SubfieldCount(4), ItemMin(4), ItemMax(4)));
					addItem(new ItemValidator(ItemCount(1), new Table12Validator));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 255)));
					addItem(new ItemValidator(ItemCount(1), new HexValidator(4, 4)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 65535)));
				finishSubfield();
			finishField();
		}

		void RecordValidator::configureType13BValidator() {
			configureType13Validator();

			addSimpleField(FieldID::TYPE_13_IMP, MANDATORY, new Table11Validator);
			//addSimpleField(FieldID::TYPE_13_IMP, MANDATORY, new NumericRangeValidator(4, 7));

			addField(new FieldValidator(FieldID::TYPE_13_FGP, MANDATORY, SubfieldMin(1), SubfieldMax(6)));
				addSubfield(new SubfieldValidator(SubfieldCount(6), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1), new NumericValueValidator(19)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_13_SPD, MANDATORY, SubfieldMin(1), SubfieldMax(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(2), ItemMax(2)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 10)));
					addItem(new ItemValidator(ItemCount(1), new Table32Validator));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_13_PPC, MANDATORY, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(6), ItemMax(6)));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("FV1|FV2|FV3|FV4|TIP")));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("NA|PRX|DST|MED")));
					addItem(new ItemValidator(ItemCount(4), new NNIntValidator));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_13_LQM, OPTIONAL, SubfieldMin(1), SubfieldMax(4)));
				addSubfield(new SubfieldValidator(SubfieldCount(4), ItemMin(4), ItemMax(4)));
					addItem(new ItemValidator(ItemCount(1), new Table12Validator));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 255)));
					addItem(new ItemValidator(ItemCount(1), new HexValidator(4, 4)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 65535)));
				finishSubfield();
			finishField();
		}

		void RecordValidator::configureType13CValidator() {
			configureType13Validator();

			addSimpleField(FieldID::TYPE_13_IMP, MANDATORY, new Table11Validator);
			//addSimpleField(FieldID::TYPE_13_IMP, MANDATORY, new NumericRangeValidator(12, 15));

			addField(new FieldValidator(FieldID::TYPE_13_FGP, MANDATORY, SubfieldMin(1), SubfieldMax(6)));
				addSubfield(new SubfieldValidator(SubfieldCount(6), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(20, 36)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_13_LQM, OPTIONAL, SubfieldMin(1), SubfieldMax(4)));
				addSubfield(new SubfieldValidator(SubfieldCount(4), ItemMin(4), ItemMax(4)));
					addItem(new ItemValidator(ItemCount(1), new Table35Validator));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 255)));
					addItem(new ItemValidator(ItemCount(1), new HexValidator(4, 4)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 65535)));
				finishSubfield();
			finishField();
		}

		void RecordValidator::configureType14Validator() {
			addSimpleField(FieldID::TYPE_14_LEN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_14_IDC, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_14_IMP, MANDATORY, new Table11Validator);
			addSimpleField(FieldID::TYPE_14_SRC, MANDATORY);
			addSimpleField(FieldID::TYPE_14_FCD, MANDATORY, new DateValidator);
			addSimpleField(FieldID::TYPE_14_HLL, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_14_VLL, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_14_SLC, MANDATORY, new MultipleValueValidator("0|1|2"));
			addSimpleField(FieldID::TYPE_14_HPS, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_14_VPS, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_14_CGA, MANDATORY, new Table1Validator);
			addSimpleField(FieldID::TYPE_14_BPX, MANDATORY, new NNIntValidator);

			addSimpleField(FieldID::TYPE_14_SHPS, OPTIONAL, new NNIntValidator);
			addSimpleField(FieldID::TYPE_14_SVPS, OPTIONAL, new NNIntValidator);

			addField(new FieldValidator(FieldID::TYPE_14_AMP, OPTIONAL, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(2), ItemMax(2)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 10)));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("XX|UP")));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_14_COM, OPTIONAL);

			addField(new FieldValidator(FieldID::TYPE_14_SEG, OPTIONAL, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(5), ItemMax(5)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 10)));
					addItem(new ItemValidator(ItemCount(4), new NNIntValidator));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_14_NQM, OPTIONAL, SubfieldMin(1), SubfieldMax(4)));
				addSubfield(new SubfieldValidator(SubfieldCount(4), ItemMin(2), ItemMax(2)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 10)));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("1|2|3|4|5|254|255")));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_14_SQM, OPTIONAL, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(4), ItemMax(4)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 10)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 255)));
					addItem(new ItemValidator(ItemCount(1), new HexValidator(4, 4)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 65535)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_14_FQM, OPTIONAL, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(4), ItemMax(4)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 10)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 255)));
					addItem(new ItemValidator(ItemCount(1), new HexValidator(4, 4)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 65535)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_14_ASEG, OPTIONAL, SubfieldMin(1), SubfieldMax(4)));
				addSubfield(new SubfieldValidator(SubfieldCount(4), ItemMin(8), ItemMax(INF)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 10)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(3, 99)));
					addItem(new ItemValidator(ItemCount(INF), new NNIntValidator));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_14_DMM, OPTIONAL, new Table27Validator);
			addSimpleField(FieldID::TYPE_14_DATA, MANDATORY, new TextLengthValidator(1));
		}

		void RecordValidator::configureType14AValidator() {
			configureType14Validator();

			addField(new FieldValidator(FieldID::TYPE_14_FGP, MANDATORY, SubfieldMin(1), SubfieldMax(6)));
				addSubfield(new SubfieldValidator(SubfieldCount(6), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 15)));
				finishSubfield();
			finishField();
		}

		void RecordValidator::configureType14BValidator() {
			configureType14Validator();

			addField(new FieldValidator(FieldID::TYPE_14_FGP, MANDATORY, SubfieldMin(1), SubfieldMax(6)));
				addSubfield(new SubfieldValidator(SubfieldCount(6), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1), new NumericValueValidator(19)));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_14_PPD, MANDATORY, SubfieldMin(1), SubfieldMax(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(2), ItemMax(2)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 10)));
					addItem(new ItemValidator(ItemCount(1), new Table32Validator));
				finishSubfield();
			finishField();

			addField(new FieldValidator(FieldID::TYPE_14_PPC, MANDATORY, SubfieldMin(1), SubfieldMax(INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(6), ItemMax(6)));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("FV1|FV2|FV3|FV4|TIP")));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("NA|PRX|DST|MED")));
					addItem(new ItemValidator(ItemCount(4), new NNIntValidator));
				finishSubfield();
			finishField();
		}

		void RecordValidator::configureType15Validator() {
			addSimpleField(FieldID::TYPE_15_LEN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_15_IDC, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_15_IMP, MANDATORY, new Table11Validator);
			addSimpleField(FieldID::TYPE_15_SRC, MANDATORY);
			addSimpleField(FieldID::TYPE_15_PCD, MANDATORY, new DateValidator);
			addSimpleField(FieldID::TYPE_15_HLL, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_15_VLL, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_15_SLC, MANDATORY, new MultipleValueValidator("0|1|2"));
			addSimpleField(FieldID::TYPE_15_HPS, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_15_VPS, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_15_CGA, MANDATORY, new Table1Validator);
			addSimpleField(FieldID::TYPE_15_BPX, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_15_PLP, MANDATORY, new Table35Validator);
			addSimpleField(FieldID::TYPE_15_SHPS, OPTIONAL, new NNIntValidator);
			addSimpleField(FieldID::TYPE_15_SVPS, OPTIONAL, new NNIntValidator);
			addSimpleField(FieldID::TYPE_15_COM, OPTIONAL);

			addField(new FieldValidator(FieldID::TYPE_15_PQM, OPTIONAL, SubfieldMin(1), SubfieldMax(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(4), ItemMax(4)));
					addItem(new ItemValidator(ItemCount(1), new Table35Validator));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 255)));
					addItem(new ItemValidator(ItemCount(1), new HexValidator(4, 4)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 65535)));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_15_DMM, OPTIONAL, new Table27Validator);
			addSimpleField(FieldID::TYPE_15_DATA, MANDATORY, new TextLengthValidator(1));
		}

		void RecordValidator::configureType16Validator() {
			addSimpleField(FieldID::TYPE_16_LEN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_16_IDC, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_16_UDI, MANDATORY);
			addSimpleField(FieldID::TYPE_16_SRC, MANDATORY);
			addSimpleField(FieldID::TYPE_16_UTD, MANDATORY, new DateValidator);
			addSimpleField(FieldID::TYPE_16_HLL, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_16_VLL, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_16_SLC, MANDATORY, new MultipleValueValidator("0|1|2"));
			addSimpleField(FieldID::TYPE_16_HPS, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_16_VPS, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_16_CGA, MANDATORY, new Table1Validator);
			addSimpleField(FieldID::TYPE_16_BPX, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_16_CSP, OPTIONAL, new Table3Validator);
			addSimpleField(FieldID::TYPE_16_SHPS, OPTIONAL, new NNIntValidator);
			addSimpleField(FieldID::TYPE_16_SVPS, OPTIONAL, new NNIntValidator);
			addSimpleField(FieldID::TYPE_16_COM, OPTIONAL);

			addField(new FieldValidator(FieldID::TYPE_16_UQS, OPTIONAL, SubfieldMin(1), SubfieldMax(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(3), ItemMax(3)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 255)));
					addItem(new ItemValidator(ItemCount(1), new HexValidator(4, 4)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 65535)));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_16_DMM, OPTIONAL, new Table27Validator);
			addSimpleField(FieldID::TYPE_16_DATA, MANDATORY, new TextLengthValidator(1));
		}

		void RecordValidator::configureType17Validator() {
			addSimpleField(FieldID::TYPE_17_LEN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_17_IDC, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_17_FID, MANDATORY, new MultipleValueValidator("0|1|2"));
			addSimpleField(FieldID::TYPE_17_SRC, MANDATORY);
			addSimpleField(FieldID::TYPE_17_ICD, MANDATORY, new DateValidator);
			addSimpleField(FieldID::TYPE_17_HLL, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_17_VLL, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_17_SLC, MANDATORY, new MultipleValueValidator("0|1|2"));
			addSimpleField(FieldID::TYPE_17_HPS, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_17_VPS, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_17_CGA, MANDATORY, new Table1Validator);
			addSimpleField(FieldID::TYPE_17_BPX, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_17_CSP, MANDATORY, new Table3Validator);
			addSimpleField(FieldID::TYPE_17_RAE, OPTIONAL, new HexValidator(4, 4));
			addSimpleField(FieldID::TYPE_17_RAU, OPTIONAL, new HexValidator(4, 4));

			addField(new FieldValidator(FieldID::TYPE_17_IPC, OPTIONAL, SubfieldMin(1), SubfieldMax(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(3), ItemMax(3)));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("0|1|2")));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("0|1|2")));
					addItem(new ItemValidator(ItemCount(1), new MultipleValueValidator("0|1|2|3")));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_17_DUI, OPTIONAL, new Type17DUIValidator);
			addSimpleField(FieldID::TYPE_17_GUI, OPTIONAL, new TextLengthValidator(16, 16));

			addField(new FieldValidator(FieldID::TYPE_17_MMS, OPTIONAL, SubfieldMin(1), SubfieldMax(1)));
				addSubfield(new SubfieldValidator(SubfieldCount(1), ItemMin(3), ItemMax(3)));
					addItem(new ItemValidator(ItemCount(3)));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_17_ECL, OPTIONAL, new Table23Validator);

			addField(new FieldValidator(FieldID::TYPE_17_COM, OPTIONAL, SubfieldMin(1), SubfieldMax(1, INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(1), ItemMax(1)));
					addItem(new ItemValidator(ItemCount(1)));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_17_SHPS, OPTIONAL, new NNIntValidator);
			addSimpleField(FieldID::TYPE_17_SVPS, OPTIONAL, new NNIntValidator);

			addField(new FieldValidator(FieldID::TYPE_17_IQS, OPTIONAL, SubfieldMin(1), SubfieldMax(1, INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(3), ItemMax(3)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 255)));
					addItem(new ItemValidator(ItemCount(1), new HexValidator(4, 4)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 65535)));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_17_ALS, OPTIONAL, new MultipleValueValidator("NIR|VIS|OTHER"));
			addSimpleField(FieldID::TYPE_17_IRD, OPTIONAL, new NNIntValidator);
			addSimpleField(FieldID::TYPE_17_DMM, OPTIONAL, new Table27Validator);
			addSimpleField(FieldID::TYPE_17_DATA, MANDATORY, new TextLengthValidator(1));
		}

		void RecordValidator::configureType99Validator() {
			addSimpleField(FieldID::TYPE_99_LEN, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_99_IDC, MANDATORY, new NNIntValidator);
			addSimpleField(FieldID::TYPE_99_SRC, MANDATORY);
			addSimpleField(FieldID::TYPE_99_BCD, MANDATORY, new UTCDateValidator);
			addSimpleField(FieldID::TYPE_99_HDV, MANDATORY, new NNIntValidator(4, 4));
			addSimpleField(FieldID::TYPE_99_BTY, MANDATORY, new Table39Validator);

			addField(new FieldValidator(FieldID::TYPE_99_BDQ, OPTIONAL, SubfieldMin(1), SubfieldMax(1, INF)));
				addSubfield(new SubfieldValidator(SubfieldCount(INF), ItemMin(3), ItemMax(3)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(0, 255)));
					addItem(new ItemValidator(ItemCount(1), new HexValidator(4, 4)));
					addItem(new ItemValidator(ItemCount(1), new NumericRangeValidator(1, 65535)));
				finishSubfield();
			finishField();

			addSimpleField(FieldID::TYPE_99_BFO, MANDATORY, new HexValidator(4, 4));
			addSimpleField(FieldID::TYPE_99_BFT, MANDATORY, new HexValidator(4, 4));
			addSimpleField(FieldID::TYPE_99_BDB, MANDATORY, new TextLengthValidator(1));
		}
	}
}
