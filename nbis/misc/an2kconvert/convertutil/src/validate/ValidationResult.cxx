#include "validate/ValidationResult.hxx"
#include "utils.hxx"

namespace convert {
	using namespace std;

	ValidationResult::ValidationResult(RecordType recordType, RecordVariant recordVariant)
	: recordType(recordType), recordVariant(recordVariant) {}

	ValidationResult::~ValidationResult() {
		deleteContents<ValidationMessage*>(warnings);
		deleteContents<ValidationMessage*>(errors);
	}

	void ValidationResult::addMessage(auto_ptr<ValidationMessage> message, ValidationLevel vl, ValidationSeverity vs) {
		if(vs == Warning || (vs == RelaxedWarning && vl == Relaxed)) {
			addWarning(message);
		} else {
			addError(message);
		}
	}

	void ValidationResult::addWarning(auto_ptr<ValidationMessage> message) {
		warnings.push_back(message.release());
	}

	void ValidationResult::addError(auto_ptr<ValidationMessage> message) {
		errors.push_back(message.release());
	}

	void ValidationResult::setRecordVariant(RecordVariant recordVariant) {
		this->recordVariant = recordVariant;
	}

	bool ValidationResult::hasWarning() const {
		if(!warnings.empty()) {
			return true;
		} else {
			return false;
		}
	}

	bool ValidationResult::hasError() const {
		if(!errors.empty()) {
			return true;
		} else {
			return false;
		}
	}

	void ValidationResult::printWarnings(ostream& ostr) const {
		for(list<ValidationMessage*>::const_iterator it = warnings.begin(); it != warnings.end(); it++) {
			ValidationMessage const& valMessage = *(*it);
			valMessage.printContext(ostr);
			ostr << "\tWarning: ";
			valMessage.printMessage(ostr);
		}
	}

	void ValidationResult::printErrors(ostream& ostr) const {
		for(list<ValidationMessage*>::const_iterator it = errors.begin(); it != errors.end(); it++) {
			ValidationMessage const& valMessage = *(*it);
			valMessage.printContext(ostr);
			ostr << "\tError: ";
			valMessage.printMessage(ostr);
		}
	}
}
