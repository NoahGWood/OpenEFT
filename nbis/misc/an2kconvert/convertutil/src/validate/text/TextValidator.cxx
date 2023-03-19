#include "validate/text/TextValidator.hxx"
#include "validate/ValidationMessage.hxx"
#include "Errors.hxx"
#include "utils.hxx"

#include <boost/regex.hpp>

#include <iostream>

namespace convert {
	using namespace std;

	TextValidator::TextValidator(ValidationSeverity vs)
	: vs(vs) {}

	TextValidator::~TextValidator() {}

	void TextValidator::validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl) {}

	TextLengthValidator::TextLengthValidator(size_t minLength, ValidationSeverity vs)
	: TextValidator(vs),
	  minLength(minLength),
	  maxLength(-1) {}

	TextLengthValidator::TextLengthValidator(size_t minLength, size_t maxLength, ValidationSeverity vs)
	: TextValidator(vs),
	  minLength(minLength),
	  maxLength(maxLength) {}

	void TextLengthValidator::validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl) {
		if(value.length() < minLength) {
			result.addMessage(auto_ptr<ValidationMessage>(new ValidationMessage(context, "value is not long enough")), vl, vs);
			throw ValidationError();
		}
		if(value.length() > maxLength) {
			result.addMessage(auto_ptr<ValidationMessage>(new ValidationMessage(context, "value is too long")), vl, vs);
			throw ValidationError();
		}
	}

	RegexValidator::RegexValidator(ValidationSeverity vs)
	: TextValidator(vs) {}

	void RegexValidator::validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl) {
		boost::regex re(getRegex(context.getFileType()));
		boost::match_results<string::const_iterator> results;
		boost::match_flag_type flags = boost::match_default;
		if(!regex_search(value.begin(), value.end(), results, re, flags)) {
			result.addMessage(auto_ptr<ValidationMessage>(new ValidationMessage(context, getErrorMessage())), vl, vs);
			throw ValidationError();
		}
	}

	Base64Validator::Base64Validator(ValidationSeverity vs)
	: RegexValidator(vs) {}

	string Base64Validator::getRegex(FileType fileType) const {
		return "^[A-Za-z0-9+/\\s]+[=]{0, 2}$";
	}

	string Base64Validator::getErrorMessage() const {
		return "value is not a base 64 string";
	}

	SingleValueValidator::SingleValueValidator(string const& value, ValidationSeverity vs)
	: TextValidator(vs),
	  sValue(value) {}

	void SingleValueValidator::validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl) {
		if(value != sValue) {
			stringstream ss;
			ss << "expected value " << sValue << ", got " << value;
			result.addMessage(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())), vl, vs);
			throw ValidationError();
		}
	}

	MultipleValueValidator::MultipleValueValidator(string const& values, ValidationSeverity vs)
	: TextValidator(vs),
	  values(values) {}

	void MultipleValueValidator::validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl) {
		auto_ptr<vector<string> > strs = split(values, '|');
		for(int i = 0; i < strs->size(); i++) {
			if(value == strs->at(i)) {
				return;
			}
		}
		result.addMessage(auto_ptr<ValidationMessage>(new ValidationMessage(context, getErrorMessage())), vl, vs);
		throw ValidationError();
	}

	string MultipleValueValidator::getErrorMessage() const {
		return "value does not match any of the allowed values";
	}

	Type3_6FGPValidator::Type3_6FGPValidator(ValidationSeverity vs)
	: MultipleValueValidator("0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|255", vs),
	  lastValue("") {}

	void Type3_6FGPValidator::validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl) {
		MultipleValueValidator::validateText(value, context, result, vl);
		if(lastValue == "" && value == "255") {
			result.addMessage(auto_ptr<ValidationMessage>(new ValidationMessage(context, "there must be at least one valid FGP value")), vl, vs);
			throw ValidationError();
		}
		if(lastValue == "255" && value != "255") {
			result.addMessage(auto_ptr<ValidationMessage>(new ValidationMessage(context, "all valid FGP values must appear before an FGP of 255")), vl, vs);
			throw ValidationError();
		}
		lastValue = value;
	}

	string Type3_6FGPValidator::getErrorMessage() const {
		return "value does not match any of the allowed values from Table 12";
	}

	ResolutionValidator::ResolutionValidator(ValidationSeverity vs)
	: RegexValidator(vs) {}

	string ResolutionValidator::getRegex(FileType fileType) const {
		return "^[0-9]{2}\\.[0-9]{2}$";
	}

	string ResolutionValidator::getErrorMessage() const {
		return "value does not have the correct format for a resolution value";
	}

	FeaturePointCodeValidator::FeaturePointCodeValidator(ValidationSeverity vs)
	: RegexValidator(vs) {}

	string FeaturePointCodeValidator::getRegex(FileType fileType) const {
		return "^[0-9]{1,2}\\.[0-9]{1,2}$";
	}

	string FeaturePointCodeValidator::getErrorMessage() const {
		return "value does not have the correct format for a feature point code";
	}

	HexValidator::HexValidator(size_t minLength, ValidationSeverity vs)
	: TextLengthValidator(minLength, -1, vs) {}

	HexValidator::HexValidator(size_t minLength, size_t maxLength, ValidationSeverity vs)
	: TextLengthValidator(minLength, maxLength, vs) {}

	void HexValidator::validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl) {
		TextLengthValidator::validateText(value, context, result, vl);
		RegexValidator::validateText(value, context, result, vl);
	}

	string HexValidator::getRegex(FileType fileType) const {
		return "^[A-Fa-f0-9]+$";
	}

	string HexValidator::getErrorMessage() const {
		return "value is not a hexadecimal number";
	}

	Type9MRCValidator::Type9MRCValidator(ValidationSeverity vs)
	: RegexValidator(vs) {}

	string Type9MRCValidator::getRegex(FileType fileType) const {
		return "^[0-9]+,[0-9]+$";
	}
	string Type9MRCValidator::getErrorMessage() const {
		return "value does not have the correct format";
	}

	Type17DUIValidator::Type17DUIValidator(ValidationSeverity vs)
	: RegexValidator(vs) {}

	string Type17DUIValidator::getRegex(FileType fileType) const {
		return "^([DMP].{15})|([0]{16})$";
	}

	string Type17DUIValidator::getErrorMessage() const {
		return "value is not a Device Unique Identifier";
	}

	Type2LCNValidator::Type2LCNValidator(ValidationSeverity vs)
	: RegexValidator(vs) {}

	string Type2LCNValidator::getRegex(FileType fileType) const {
		return "^[0-9a-zA-Z]{2}-[0-9a-zA-Z]{8}$";
	}

	string Type2LCNValidator::getErrorMessage() const {
		return "value is not an FBI Latent Case Number";
	}
}
