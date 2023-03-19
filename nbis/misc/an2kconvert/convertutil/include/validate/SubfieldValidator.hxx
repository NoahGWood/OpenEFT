#ifndef SUBFIELDVALIDATOR_HXX
#define SUBFIELDVALIDATOR_HXX

#include "validate/Context.hxx"
#include "validate/ItemValidator.hxx"
#include "validate/ValidationResult.hxx"
#include "validate/Validation.hxx"
#include "part1/Subfield.hxx"

#include <list>
#include <memory>
#include <string>

namespace convert {
	namespace part1 {
		using namespace std;

		class SubfieldCount {
		public:
			SubfieldCount(size_t value) : value(value) {};
			size_t value;
		};

		class ItemMin {
		public:
			ItemMin(size_t value) : value1(value), value2(value) {};
			ItemMin(size_t value1, size_t value2) : value1(value1), value2(value2) {};
			size_t value1;
			size_t value2;
		};

		class ItemMax {
		public:
			ItemMax(size_t value) : value1(value), value2(value) {};
			ItemMax(size_t value1, size_t value2) : value1(value1), value2(value2) {};
			size_t value1;
			size_t value2;
		};

		class SubfieldValidator {
		public:
			SubfieldValidator(SubfieldCount subfieldCount, ItemMin itemMin);
			SubfieldValidator(SubfieldCount subfieldCount, ItemMin itemMin, ItemMax itemMax);
			~SubfieldValidator();
			size_t getSubfieldCount() const;
			void addValidator(auto_ptr<ItemValidator> validator);
			void validateSubfield(Subfield const& subfield, Context& context, ValidationResult& valResult, ValidationLevel vl) const;

		private:
			void validateItemCount(Subfield const& subfield, Context& context, ValidationResult& valResult) const;

			size_t subfieldCount;
			size_t itemMin1;
			size_t itemMin2;
			size_t itemMax1;
			size_t itemMax2;
			list<ItemValidator*> itemValidators;
		};
	}
}

#endif
