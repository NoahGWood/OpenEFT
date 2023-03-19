#ifndef ERRORS_HXX
#define ERRORS_HXX

#include <stdexcept>

namespace convert {
	using namespace std;

	class IOError : public runtime_error {
	public:
		IOError(string const& what_arg) : runtime_error(what_arg) {}
	};

	class ParseError : public runtime_error {
	public:
		ParseError(string const& what_arg) : runtime_error(what_arg) {}
	};

	class ValidationError : public runtime_error {
	public:
		ValidationError() : runtime_error("") {}
		ValidationError(string const& what_arg) : runtime_error(what_arg) {}
	};
}

#endif
