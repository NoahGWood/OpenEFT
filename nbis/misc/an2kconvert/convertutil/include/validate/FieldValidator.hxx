#ifndef FIELDVALIDATOR_HXX
#define FIELDVALIDATOR_HXX

#include "validate/SubfieldValidator.hxx"
#include "validate/ValidationResult.hxx"
#include "validate/Context.hxx"
#include "validate/Validation.hxx"
#include "part1/Field.hxx"
#include "part1/FieldID.hxx"


#include <list>
#include <memory>
#include <string>

namespace convert {
	namespace part1 {
		using namespace std;

		enum FieldRequired {
			MANDATORY,
			OPTIONAL_WARNING,
			OPTIONAL
		};

		class SubfieldMin {
		public:
			SubfieldMin(size_t value) : value1(value), value2(value) {};
			SubfieldMin(size_t value1, size_t value2) : value1(value1), value2(value2) {};
			size_t value1;
			size_t value2;
		};

		class SubfieldMax {
		public:
			SubfieldMax(size_t value) : value1(value), value2(value) {};
			SubfieldMax(size_t value1, size_t value2) : value1(value1), value2(value2) {};
			size_t value1;
			size_t value2;
		};

		class FieldValidator {
		public:
			FieldValidator(FieldID const& fieldId, FieldRequired fieldRequired, SubfieldMin subfieldMin);
			FieldValidator(FieldID const& fieldId, FieldRequired fieldRequired, SubfieldMin subfieldMin, SubfieldMax subfieldMax);
			~FieldValidator();
			FieldID const& getFieldId() const;
			void addValidator(auto_ptr<SubfieldValidator> validator);
			void validateField(Field const& field, Context& context, ValidationResult& valResult, ValidationLevel vl) const;

		private:
			void validateSubfieldCount(Field const& field, Context& context, ValidationResult& valResult) const;

			FieldID fieldId;
			FieldRequired fieldRequired;
			size_t subfieldMin1;
			size_t subfieldMin2;
			size_t subfieldMax1;
			size_t subfieldMax2;
			list<SubfieldValidator*> subfieldValidators;
		};
	}
}

#endif

