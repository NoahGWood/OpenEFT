#ifndef PART1_RECORDVALIDATOR_HXX
#define PART1_RECORDVALIDATOR_HXX

#include "RecordType.hxx"
#include "part1/FieldID.hxx"
#include "part1/Record.hxx"
#include "validate/FieldValidator.hxx"
#include "validate/ItemValidator.hxx"
#include "validate/SubfieldValidator.hxx"
#include "validate/text/DateValidators.hxx"
#include "validate/text/NumericValidators.hxx"
#include "validate/text/TextValidator.hxx"
#include "validate/text/TableValidators.hxx"

#include <list>
#include <memory>
#include <stack>
#include <string>

namespace convert {
	namespace part1 {
		using namespace std;

		class RecordValidator {
		public:
			~RecordValidator();
			static RecordVariant validateRecord(Record const& part1Record, ValidationLevel vl, bool ebts);

		private:
			explicit RecordValidator(RecordType recordType, RecordVariant recordVariant);
			void addSimpleField(FieldID const& fieldId, FieldRequired fieldRequired, TextValidator* textValidator = new TextValidator);
			void addField(FieldValidator* validator);
			void finishField();
			void addSubfield(SubfieldValidator* validator);
			void finishSubfield();
			void addItem(ItemValidator* validator);
			bool hasFieldValidator(FieldID const& fieldId) const;
			auto_ptr<ValidationResult> validate(Record const& part1Record, ValidationLevel vl);
			void configureType1Validator();
			void configureType2AValidator();
			void configureType2BValidator();
			void configureType3_6Validator(RecordType recordType);
			void configureType7Validator();
			void configureType8Validator();
			void configureType8AValidator();
			void configureType8BValidator();
			void configureType9AValidator();
			void configureType9BValidator();
			void configureType9CValidator();
			void configureType9DValidator();
			void configureType10AValidator();
			void configureType10BValidator();
			void configureType13Validator();
			void configureType13AValidator();
			void configureType13BValidator();
			void configureType13CValidator();
			void configureType14Validator();
			void configureType14AValidator();
			void configureType14BValidator();
			void configureType15Validator();
			void configureType16Validator();
			void configureType17Validator();
			void configureType99Validator();

			RecordType recordType;
			RecordVariant recordVariant;
			list<FieldValidator*> fieldValidators;
			auto_ptr<FieldValidator> currField;
			auto_ptr<SubfieldValidator> currSubfield;
		};
	}
}

#endif
