#include "validate/text/NumericValidators.hxx"
#include "validate/ValidationMessage.hxx"
#include "Errors.hxx"
#include "utils.hxx"

#include <boost/regex.hpp>

#include <iostream>

namespace convert {
	using namespace std;

	IntValidator::IntValidator(ValidationSeverity vs)
	: RegexValidator(vs) {}

	void IntValidator::validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl) {
		RegexValidator::validateText(value, context, result, vl);
	}

	string IntValidator::getRegex(FileType fileType) const {
		return "^[-]{0,1}[0-9]+$";
	}

	string IntValidator::getErrorMessage() const {
		return "value is not an integer";
	}

	NNIntValidator::NNIntValidator(ValidationSeverity vs)
	: TextLengthValidator(0, -1, vs) {}

	NNIntValidator::NNIntValidator(size_t minLength, ValidationSeverity vs)
	: TextLengthValidator(minLength, -1, vs) {}

	NNIntValidator::NNIntValidator(size_t minLength, size_t maxLength, ValidationSeverity vs)
	: TextLengthValidator(minLength, maxLength, vs) {}

	void NNIntValidator::validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl) {
		TextLengthValidator::validateText(value, context, result, vl);
		RegexValidator::validateText(value, context, result, vl);
	}

	string NNIntValidator::getRegex(FileType fileType) const {
		return "^[0-9]+$";
	}

	string NNIntValidator::getErrorMessage() const {
		return "value is not a non-negative integer";
	}

	NumericValueValidator::NumericValueValidator(unsigned int value, ValidationSeverity vs)
	: IntValidator(vs),
	  numValue(value) {}

	void NumericValueValidator::validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl) {
		IntValidator::validateText(value, context, result, vl);
		if(stringToUInt(value) != numValue) {
			stringstream ss;
			ss << "expected value " << numValue << ", got " << value;
			result.addMessage(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())), vl, vs);
			throw ValidationError();
		}
	}

	NumericRangeValidator::NumericRangeValidator(unsigned int minValue, unsigned int maxValue, ValidationSeverity vs)
	: IntValidator(vs),
	  minValue(minValue),
	  maxValue(maxValue) {}

	void NumericRangeValidator::validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl) {
		IntValidator::validateText(value, context, result, vl);
		unsigned int numValue = stringToUInt(value);
		if(numValue < minValue || numValue > maxValue) {
			stringstream ss;
			ss << "expected value between " << minValue << " and " << maxValue << ", got " << value;
			result.addMessage(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())), vl, vs);
			throw ValidationError();
		}
	}

	MultipleNumericValidator::MultipleNumericValidator(string const& values, ValidationSeverity vs)
	: IntValidator(vs),
	  values(values) {}

	void MultipleNumericValidator::validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl) {
		IntValidator::validateText(value, context, result, vl);
		auto_ptr<vector<string> > strs = split(values, '|');
		for(int i = 0; i < strs->size(); i++) {
			if(stringToInt(value) == stringToInt(strs->at(i))) {
				return;
			}
		}
		result.addMessage(auto_ptr<ValidationMessage>(new ValidationMessage(context, getErrorMessage())), vl, vs);
		throw ValidationError();
	}

	string MultipleNumericValidator::getErrorMessage() const {
		return "value does not match any of the allowed values";
	}

	IntRangeValidator::IntRangeValidator(int minValue, int maxValue, ValidationSeverity vs)
	: IntValidator(vs),
	  minValue(minValue),
	  maxValue(maxValue) {}

	void IntRangeValidator::validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl) {
		IntValidator::validateText(value, context, result, vl);
		int numValue = stringToInt(value);
		if(numValue < minValue || numValue > maxValue) {
			stringstream ss;
			ss << "expected value between " << minValue << " and " << maxValue << ", got " << value;
			result.addMessage(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())), vl, vs);
			throw ValidationError();
		}
	}

	U8Validator::U8Validator(ValidationSeverity vs)
	: NumericRangeValidator(0, 0xFF, vs) {}

	U16Validator::U16Validator(ValidationSeverity vs)
	: NumericRangeValidator(0, 0xFFFF, vs) {}

	U32Validator::U32Validator(ValidationSeverity vs)
	: NumericRangeValidator(0, 0xFFFFFFFF, vs) {}
}
