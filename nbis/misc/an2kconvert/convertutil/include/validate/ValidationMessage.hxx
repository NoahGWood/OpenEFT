#ifndef VALIDATIONMESSAGE_HXX
#define VALIDATIONMESSAGE_HXX

#include "validate/Context.hxx"

#include <ostream>
#include <string>

namespace convert {
	using namespace std;

	class ValidationMessage {
	public:
		ValidationMessage(Context const& context, string const& message);
		void printContext(ostream& ostr) const;
		void printMessage(ostream& ostr) const;

	private:
		Context context;
		string message;
	};
}

#endif
