#ifndef VALIDATIONRESULT_HXX
#define VALIDATIONRESULT_HXX

#include "RecordType.hxx"
#include "validate/ValidationMessage.hxx"
#include "validate/Validation.hxx"

#include <list>
#include <memory>
#include <ostream>

namespace convert {
	using namespace std;

	class ValidationResult {
	public:
		ValidationResult(RecordType recordType, RecordVariant recordVariant);
		~ValidationResult();
		void addMessage(auto_ptr<ValidationMessage> message, ValidationLevel vl, ValidationSeverity vs);
		void addWarning(auto_ptr<ValidationMessage> message);
		void addError(auto_ptr<ValidationMessage> message);
		void setRecordVariant(RecordVariant recordVariant);
		bool hasWarning() const;
		bool hasError() const;
		void printWarnings(ostream& ostr) const;
		void printErrors(ostream& ostr) const;

	private:
		RecordType recordType;
		RecordVariant recordVariant;
		list<ValidationMessage*> warnings;
		list<ValidationMessage*> errors;
	};
}

#endif
