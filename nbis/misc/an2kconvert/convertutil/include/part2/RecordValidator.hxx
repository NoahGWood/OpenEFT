#ifndef PART2_RECORDVALIDATOR_HXX
#define PART2_RECORDVALIDATOR_HXX

#include "RecordType.hxx"
#include "part2/Record.hxx"
#include "validate/ElementValidator.hxx"
#include "validate/ValidationResult.hxx"
#include "validate/text/DateValidators.hxx"
#include "validate/text/NumericValidators.hxx"
#include "validate/text/TextValidator.hxx"
#include "validate/text/TableValidators.hxx"

#include <list>
#include <memory>
#include <stack>
#include <string>

namespace convert {
	namespace part2 {

	using namespace std;

		class RecordValidator {
		public:
			~RecordValidator();
			static RecordVariant validateRecord(Record const& part2Record, ValidationLevel vl, bool ebts);

		private:
			explicit RecordValidator(RecordType recordType, RecordVariant recordVariant);
			void addNode(ElementValidator* validator);
			void finishNode(string const& elementName, XMLNamespace ns);
			void addLeafNode(ElementValidator* validator);
			auto_ptr<ValidationResult> validate(Record const& part2Record, ValidationLevel vl);
			void configureType1AValidator();
			void configureType1BValidator();
			void configureType2AValidator();
			void configureType2BValidator();
			void configureType3Validator();
			void configureType4Validator();
			void configureType5Validator();
			void configureType6Validator();
			void configureType3_6Validator();
			void configureType7Validator();
			void configureType8AValidator();
			void configureType8BValidator();
			void configureType9AValidator();
			void configureType9BValidator();
			void configureType9CValidator();
			void configureType9DValidator();
			void configureType10AValidator();
			void configureType10BValidator();
			void configureType13AValidator();
			void configureType13BValidator();
			void configureType13CValidator();
			void configureType14AValidator();
			void configureType14BValidator();
			void configureType15Validator();
			void configureType16Validator();
			void configureType17Validator();
			void configureType99Validator();

			RecordType recordType;
			RecordVariant recordVariant;
			auto_ptr<ElementValidator> rootNode;
			stack<ElementValidator*, list<ElementValidator*> > nodeStack;
		};
	}
}

#endif
