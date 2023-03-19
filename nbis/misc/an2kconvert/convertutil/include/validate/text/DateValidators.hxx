#ifndef DATEVALIDATORS_HXX
#define DATEVALIDATORS_HXX

#include "validate/text/TextValidator.hxx"
#include "validate/ValidationResult.hxx"
#include "validate/Context.hxx"
#include "part1/FieldID.hxx"
#include "part1/Item.hxx"

namespace convert {
	using namespace std;

	class DateValidator : public RegexValidator {
	public:
		DateValidator(ValidationSeverity vs = Error);
		void validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl);
	private:
		string getRegex(FileType fileType) const;
		string getErrorMessage() const;
	};

	class UTCDateValidator : public RegexValidator {
	public:
		UTCDateValidator(ValidationSeverity vs = Error);
		void validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl);
	private:
		string getRegex(FileType fileType) const;
		string getErrorMessage() const;
	};
}

#endif
