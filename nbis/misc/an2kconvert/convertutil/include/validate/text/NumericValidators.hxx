#ifndef NUMERICVALIDATORS_HXX
#define NUMERICVALIDATORS_HXX

#include "validate/text/TextValidator.hxx"
#include "validate/ValidationResult.hxx"
#include "validate/Context.hxx"
#include "part1/FieldID.hxx"
#include "part1/Item.hxx"

namespace convert {
	using namespace std;

	class IntValidator : public RegexValidator {
	public:
		IntValidator(ValidationSeverity vs = Error);
		virtual void validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl);
	private:
		string getRegex(FileType fileType) const;
		string getErrorMessage() const;
	};

	class NNIntValidator : public TextLengthValidator, public RegexValidator {
	public:
		NNIntValidator(ValidationSeverity vs = Error);
		NNIntValidator(size_t minLength, ValidationSeverity vs = Error);
		NNIntValidator(size_t minLength, size_t maxLength, ValidationSeverity vs = Error);
		void validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl);
	private:
		string getRegex(FileType fileType) const;
		string getErrorMessage() const;
	};

	class NumericValueValidator : public IntValidator {
	public:
		NumericValueValidator(unsigned int value, ValidationSeverity vs = Error);
		void validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl);

	private:
		unsigned int numValue;
	};

	class NumericRangeValidator : public IntValidator {
	public:
		NumericRangeValidator(unsigned int minValue, unsigned int maxValue, ValidationSeverity vs = Error);
		void validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl);

	private:
		unsigned int minValue;
		unsigned int maxValue;
	};

	class MultipleNumericValidator : public IntValidator {
	public:
		MultipleNumericValidator(string const& values, ValidationSeverity vs = Error);
		void validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl);
	private:
		virtual string getErrorMessage() const;
		string values;
	};

	class IntRangeValidator : public IntValidator {
	public:
		IntRangeValidator(int minValue, int maxValue, ValidationSeverity vs = Error);
		void validateText(string const& value, Context& context, ValidationResult& result, ValidationLevel vl);

	private:
		int minValue;
		int maxValue;
	};

	class U8Validator : public NumericRangeValidator {
	public:
		U8Validator(ValidationSeverity vs = Error);
	};

	class U16Validator : public NumericRangeValidator {
	public:
		U16Validator(ValidationSeverity vs = Error);
	};

	class U32Validator : public NumericRangeValidator {
	public:
		U32Validator(ValidationSeverity vs = Error);
	};
}

#endif
