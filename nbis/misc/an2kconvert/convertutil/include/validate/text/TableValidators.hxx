#ifndef TABLEVALIDATORS_HXX
#define TABLEVALIDATORS_HXX

#include "validate/text/TextValidator.hxx"
#include "validate/text/NumericValidators.hxx"
#include "validate/ValidationResult.hxx"
#include "validate/Context.hxx"
#include "part1/FieldID.hxx"
#include "part1/Item.hxx"

namespace convert {
	using namespace std;

	class Table1Validator : public MultipleValueValidator {
	public:
		Table1Validator();
	private:
		string getErrorMessage() const;
	};

	class Table3Validator : public MultipleValueValidator {
	public:
		Table3Validator();
	private:
		string getErrorMessage() const;
	};

	class Table11Validator : public MultipleNumericValidator {
	public:
		Table11Validator();
	private:
		string getErrorMessage() const;
	};

	class Table12Validator : public MultipleNumericValidator {
	public:
		Table12Validator();
	private:
		string getErrorMessage() const;
	};

	class Table15Validator : public MultipleValueValidator {
	public:
		Table15Validator();
	private:
		string getErrorMessage() const;
	};

	class Table18Validator : public MultipleNumericValidator {
	public:
		Table18Validator();
	private:
		string getErrorMessage() const;
	};

	class Table19Validator : public MultipleValueValidator {
	public:
		Table19Validator();
	private:
		string getErrorMessage() const;
	};

	class Table20Validator : public MultipleValueValidator {
	public:
		Table20Validator();
	private:
		string getErrorMessage() const;
	};

	class Table21Validator : public MultipleValueValidator {
	public:
		Table21Validator();
	private:
		string getErrorMessage() const;
	};

	class Table22Validator : public MultipleValueValidator {
	public:
		Table22Validator();
	private:
		string getErrorMessage() const;
	};

	class Table23Validator : public MultipleValueValidator {
	public:
		Table23Validator();
	private:
		string getErrorMessage() const;
	};

	class Table24Validator : public MultipleValueValidator {
	public:
		Table24Validator();
	private:
		string getErrorMessage() const;
	};

	class Table27Validator : public MultipleValueValidator {
	public:
		Table27Validator();
	private:
		string getErrorMessage() const;
	};

	class Table28Validator : public MultipleValueValidator {
	public:
		Table28Validator();
	private:
		string getErrorMessage() const;
	};

	class Table29Validator : public MultipleValueValidator {
	public:
		Table29Validator();
	private:
		string getErrorMessage() const;
	};

	class Table30Validator : public MultipleValueValidator {
	public:
		Table30Validator();
	private:
		string getErrorMessage() const;
	};

	class Table32Validator : public MultipleValueValidator {
	public:
		Table32Validator();
	private:
		string getErrorMessage() const;
	};

	class Table35Validator : public MultipleNumericValidator {
	public:
		Table35Validator();
	private:
		string getErrorMessage() const;
	};

	class Table39Validator : public MultipleValueValidator {
	public:
		Table39Validator();
	private:
		string getErrorMessage() const;
	};

	class Table201Validator : public MultipleValueValidator {
	public:
		Table201Validator();
	private:
		string getErrorMessage() const;
	};

	class Table203Validator : public MultipleValueValidator {
	public:
		Table203Validator();
	private:
		string getErrorMessage() const;
	};

	class Table211Validator : public MultipleNumericValidator {
	public:
		Table211Validator();
	private:
		string getErrorMessage() const;
	};

	class Table212Validator : public MultipleNumericValidator {
	public:
		Table212Validator();
	private:
		string getErrorMessage() const;
	};

	class Table215Validator : public MultipleValueValidator {
	public:
		Table215Validator();
	private:
		string getErrorMessage() const;
	};

	class Table219Validator : public MultipleValueValidator {
	public:
		Table219Validator();
	private:
		string getErrorMessage() const;
	};

	class Table220Validator : public MultipleNumericValidator {
	public:
		Table220Validator();
	private:
		string getErrorMessage() const;
	};

	class Table221Validator : public MultipleValueValidator {
	public:
		Table221Validator();
	private:
		string getErrorMessage() const;
	};

	class Table223Validator : public MultipleValueValidator {
	public:
		Table223Validator();
	private:
		string getErrorMessage() const;
	};

	class Table226Validator : public MultipleValueValidator {
	public:
		Table226Validator();
	private:
		string getErrorMessage() const;
	};

	class Table227Validator : public MultipleValueValidator {
	public:
		Table227Validator();
	private:
		string getErrorMessage() const;
	};

	class Table228Validator : public MultipleValueValidator {
	public:
		Table228Validator();
	private:
		string getErrorMessage() const;
	};

	class Table229Validator : public MultipleValueValidator {
	public:
		Table229Validator();
	private:
		string getErrorMessage() const;
	};

	class Table230Validator : public MultipleValueValidator {
	public:
		Table230Validator();
	private:
		string getErrorMessage() const;
	};

	class Table231Validator : public MultipleValueValidator {
	public:
		Table231Validator();
	private:
		string getErrorMessage() const;
	};

	class Table233Validator : public MultipleValueValidator {
	public:
		Table233Validator();
	private:
		string getErrorMessage() const;
	};

	class Table235Validator : public MultipleNumericValidator {
	public:
		Table235Validator();
	private:
		string getErrorMessage() const;
	};

	class Table240Validator : public MultipleValueValidator {
	public:
		Table240Validator();
	private:
		string getErrorMessage() const;
	};
}

#endif
