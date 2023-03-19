#ifndef VALIDATION_HXX
#define VALIDATION_HXX

namespace convert {
	using namespace std;

	enum ValidationLevel {
		Relaxed,
		Strict
	};

	enum ValidationSeverity {
		Warning,
		RelaxedWarning,
		Error
	};
}

#endif
