#include "validate/FieldValidator.hxx"
#include "Errors.hxx"
#include "utils.hxx"

namespace convert {
	namespace part1 {
		using namespace std;

		FieldValidator::FieldValidator(FieldID const& fieldId, FieldRequired fieldRequired, SubfieldMin subfieldMin)
		: fieldId(fieldId),
		  fieldRequired(fieldRequired),
		  subfieldMin1(subfieldMin.value1),
		  subfieldMin2(subfieldMin.value2),
		  subfieldMax1(INF),
		  subfieldMax2(INF) {}

		FieldValidator::FieldValidator(FieldID const& fieldId, FieldRequired fieldRequired, SubfieldMin subfieldMin, SubfieldMax subfieldMax)
		: fieldId(fieldId),
		  fieldRequired(fieldRequired),
		  subfieldMin1(subfieldMin.value1),
		  subfieldMin2(subfieldMin.value2),
		  subfieldMax1(subfieldMax.value1),
		  subfieldMax2(subfieldMax.value2){}

		FieldValidator::~FieldValidator() {
			deleteContents<SubfieldValidator*>(subfieldValidators);
		}

		FieldID const& FieldValidator::getFieldId() const {
			return fieldId;
		}

		void FieldValidator::addValidator(auto_ptr<SubfieldValidator> validator) {
			subfieldValidators.push_back(validator.release());
		}

		void FieldValidator::validateField(Field const& field, Context& context, ValidationResult& valResult, ValidationLevel vl) const {
			if(field.getFieldID() == FieldID::MISSING_FIELD) {
				if(fieldRequired == MANDATORY) {
					valResult.addError(auto_ptr<ValidationMessage>(new ValidationMessage(context, "required field is missing")));
					throw ValidationError();
				}
				if(fieldRequired == OPTIONAL_WARNING) {
					valResult.addWarning(auto_ptr<ValidationMessage>(new ValidationMessage(context, "field is missing")));
					return;
				}
				if(fieldRequired == OPTIONAL) {
					return;
				}
			}

			validateSubfieldCount(field, context, valResult);

			size_t subfieldIndex = 0;
			for(list<SubfieldValidator*>::const_iterator it = subfieldValidators.begin(); it != subfieldValidators.end(); it++) {
				SubfieldValidator const& val = *(*it);
				size_t subfieldCount = 0;
				while(subfieldIndex < field.subfieldsCount() && subfieldCount < val.getSubfieldCount()) {
					context.setSubfieldIndex(subfieldIndex + 1);
					val.validateSubfield(field.getSubfield(subfieldIndex), context, valResult, vl);
					context.unsetSubfieldIndex();
					subfieldCount++;
					subfieldIndex++;
				}
			}
		}

		void FieldValidator::validateSubfieldCount(Field const& field, Context& context, ValidationResult& valResult) const {
			if(field.subfieldsCount() < subfieldMin1) {
				//if subfieldsCount is less than both min values (min1 should be <= min2), throw an error
				stringstream ss;
				ss << "not enough subfields in field, min " << subfieldMin1 << ", got " << field.subfieldsCount();
				valResult.addError(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())));
				throw ValidationError();
			} else if(field.subfieldsCount() < subfieldMin2) {
				//if subfieldsCount is only less than one of the min values, give a warning
				stringstream ss;
				ss << "not enough subfields in field, min " << subfieldMin2 << ", got " << field.subfieldsCount();
				valResult.addWarning(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())));
			}

			if(field.subfieldsCount() > subfieldMax2) {
				//if subfieldsCount is greater than both max values (max2 should be >= max1), throw an error
				stringstream ss;
				ss << "too many subfields in field, max " << subfieldMax2 << ", got " << field.subfieldsCount();
				valResult.addError(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())));
				throw ValidationError();
			} else if(field.subfieldsCount() > subfieldMax1) {
				//if subfieldsCount is only greater than one of the max values, give a warning
				stringstream ss;
				ss << "too many subfields in field, max " << subfieldMax1 << ", got " << field.subfieldsCount();
				valResult.addWarning(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())));
			}
		}
	}
}
