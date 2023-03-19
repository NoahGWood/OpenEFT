#include "validate/text/DateValidators.hxx"
#include "validate/ValidationMessage.hxx"
#include "Errors.hxx"
#include "utils.hxx"

#include <boost/regex.hpp>

#include <iostream>

namespace convert {
	using namespace std;

	DateValidator::DateValidator(ValidationSeverity vs)
	: RegexValidator(vs) {}

	void DateValidator::validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl) {
		RegexValidator::validateText(value, context, result, vl);
		int year, month, day;
		if(isPart1(context.getFileType())) {
			year = stringToInt(value.substr(0, 4));
			month = stringToInt(value.substr(4, 2));
			day = stringToInt(value.substr(6, 2));
		}
		if(isPart2(context.getFileType())) {
			year = stringToInt(value.substr(0, 4));
			month = stringToInt(value.substr(5, 2));
			day = stringToInt(value.substr(8, 2));
		}
		bool isLeapYear = year % 4 == 0 && !(year % 100 == 0);
		if(month < 1 || month > 12) {
			result.addMessage(auto_ptr<ValidationMessage>(new ValidationMessage(context, "value is not a valid date")), vl, vs);
			throw ValidationError();
		}
		int maxDay;
		switch(month) {
		case 1:	case 3:	case 5:	case 7:	case 8:	case 10: case 12:
			maxDay = 31;
			break;
		case 4:	case 6:	case 9:	case 11:
			maxDay = 30;
			break;
		case 2:
			isLeapYear ? maxDay = 29 : maxDay = 28;
			break;
		}
		if(day < 1 || day > maxDay) {
			result.addMessage(auto_ptr<ValidationMessage>(new ValidationMessage(context, "value is not a valid date")), vl, vs);
			throw ValidationError();
		}
	}

	string DateValidator::getRegex(FileType fileType) const {
		if(isPart1(fileType)) {
			return "^[0-9]{8}$";
		}
		if(isPart2(fileType)) {
			return "^[0-9]{4}-[0-9]{2}-[0-9]{2}$";
		}
	}

	string DateValidator::getErrorMessage() const {
		return "value is not a date";
	}

	UTCDateValidator::UTCDateValidator(ValidationSeverity vs)
	: RegexValidator(vs) {}

	void UTCDateValidator::validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl) {
		RegexValidator::validateText(value, context, result, vl);
		int year, month, day, hour, minute, second;
		if(isPart1(context.getFileType())) {
			year = stringToInt(value.substr(0, 4));
			month = stringToInt(value.substr(4, 2));
			day = stringToInt(value.substr(6, 2));
			hour = stringToInt(value.substr(8, 2));
			minute = stringToInt(value.substr(10, 2));
			second = stringToInt(value.substr(12, 2));
		}
		if(isPart2(context.getFileType())) {
			year = stringToInt(value.substr(0, 4));
			month = stringToInt(value.substr(5, 2));
			day = stringToInt(value.substr(8, 2));
			hour = stringToInt(value.substr(11, 2));
			minute = stringToInt(value.substr(14, 2));
			second = stringToInt(value.substr(17, 2));
		}
		bool isLeapYear = year % 4 == 0 && !(year % 100 == 0);
		if(month < 1 || month > 12) {
			result.addError(auto_ptr<ValidationMessage>(new ValidationMessage(context, "value is not a valid UTC date")));
			throw ValidationError();
		}
		int maxDay;
		switch(month) {
		case 1:	case 3:	case 5:	case 7:	case 8:	case 10: case 12:
			maxDay = 31;
			break;
		case 4:	case 6:	case 9:	case 11:
			maxDay = 30;
			break;
		case 2:
			isLeapYear ? maxDay = 29 : maxDay = 28;
			break;
		}
		if(day < 1 || day > maxDay) {
			result.addError(auto_ptr<ValidationMessage>(new ValidationMessage(context, "value is not a valid UTC date")));
			throw ValidationError();
		}
		if(hour > 23 || minute > 59 || second > 59) {
			result.addError(auto_ptr<ValidationMessage>(new ValidationMessage(context, "value is not a valid UTC date")));
			throw ValidationError();
		}
	}

	string UTCDateValidator::getRegex(FileType fileType) const {
		if(isPart1(fileType)) {
			return "^[0-9]{14}Z$";
		}
		if(isPart2(fileType)) {
			return "^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}Z$";
		}
	}

	string UTCDateValidator::getErrorMessage() const {
		return "value is not a UTC date";
	}
}
