#ifndef TEXTVALIDATOR_HXX
#define TEXTVALIDATOR_HXX

#include "validate/Validation.hxx"
#include "validate/ValidationResult.hxx"
#include "validate/Context.hxx"
#include "part1/FieldID.hxx"
#include "part1/Item.hxx"
#include "ITLPackage.hxx"

namespace convert {
	using namespace std;

	class TextValidator {
	public:
		TextValidator(ValidationSeverity vs = Error);
		virtual ~TextValidator();
		virtual void validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl);
	protected:
		ValidationSeverity vs;
	};

	class TextLengthValidator : public virtual TextValidator {
	public:
		TextLengthValidator(size_t minLength, ValidationSeverity vs = Error);
		TextLengthValidator(size_t minLength, size_t maxLength, ValidationSeverity vs = Error);
		virtual void validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl);

	private:
		size_t minLength;
		size_t maxLength;
	};

	class RegexValidator : public virtual TextValidator {
	public:
		RegexValidator(ValidationSeverity vs = Error);
		virtual void validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl);

	protected:
		virtual string getRegex(FileType fileType) const = 0;
		virtual string getErrorMessage() const = 0;
	};

	class Base64Validator : public RegexValidator {
	public:
		Base64Validator(ValidationSeverity vs = Error);
	private:
		string getRegex(FileType fileType) const;
		string getErrorMessage() const;
	};

	class SingleValueValidator : public TextValidator {
	public:
		SingleValueValidator(string const& value, ValidationSeverity vs = Error);
		void validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl);

	private:
		string sValue;
	};

	class MultipleValueValidator : public TextValidator {
	public:
		MultipleValueValidator(string const& values, ValidationSeverity vs = Error);
		virtual void validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl);

	private:
		virtual string getErrorMessage() const;
		string values;
	};

	class Type3_6FGPValidator : public MultipleValueValidator {
	public:
		Type3_6FGPValidator(ValidationSeverity vs = Error);
		void validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl);
	private:
		string getErrorMessage() const;
		string lastValue;
	};

	class ResolutionValidator : public RegexValidator {
	public:
		ResolutionValidator(ValidationSeverity vs = Error);
	private:
		string getRegex(FileType fileType) const;
		string getErrorMessage() const;
	};

	class FeaturePointCodeValidator : public RegexValidator {
	public:
		FeaturePointCodeValidator(ValidationSeverity vs = Error);
	private:
		string getRegex(FileType fileType) const;
		string getErrorMessage() const;
	};

	class HexValidator : public TextLengthValidator, public RegexValidator {
	public:
		HexValidator(size_t minLength, ValidationSeverity vs = Error);
		HexValidator(size_t minLength, size_t maxLength, ValidationSeverity vs = Error);
		void validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl);
	private:
		string getRegex(FileType fileType) const;
		string getErrorMessage() const;
	};

	class Type9MRCValidator : public RegexValidator {
	public:
		Type9MRCValidator(ValidationSeverity vs = Error);
	private:
		string getRegex(FileType fileType) const;
		string getErrorMessage() const;
	};

	class Type17DUIValidator : public RegexValidator {
	public:
		Type17DUIValidator(ValidationSeverity vs = Error);
	private:
		string getRegex(FileType fileType) const;
		string getErrorMessage() const;
	};

	class Type2LCNValidator : public RegexValidator {
	public:
		Type2LCNValidator(ValidationSeverity vs = Error);
	private:
		string getRegex(FileType fileType) const;
		string getErrorMessage() const;
	};
}

#endif
