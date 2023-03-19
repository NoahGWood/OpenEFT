#include "validate/ValidationMessage.hxx"

#include <iostream>

namespace convert {
	using namespace std;

	ValidationMessage::ValidationMessage(Context const& context, string const& message)
	: context(context),
	  message(message) {}

	void ValidationMessage::printContext(ostream& ostr) const {
		context.printContext(ostr);
	}

	void ValidationMessage::printMessage(ostream& ostr) const {
		ostr << message << endl;
	}
}
