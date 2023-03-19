#ifndef ITEMVALIDATOR_HXX
#define ITEMVALIDATOR_HXX

#include "validate/text/TextValidator.hxx"
#include "validate/ValidationResult.hxx"
#include "validate/Context.hxx"
#include "validate/Validation.hxx"
#include "part1/Item.hxx"

#include <list>
#include <memory>
#include <string>

namespace convert {
	namespace part1 {
		using namespace std;

		class ItemCount {
		public:
			ItemCount(size_t value) : value(value) {};
			size_t value;
		};

		class ItemValidator {
		public:
			ItemValidator(ItemCount itemCount, TextValidator* textValidator = new TextValidator);
			size_t getItemCount() const;
			void validateItem(Item const& item, Context& context, ValidationResult& valResult, ValidationLevel vl) const;

		private:
			size_t itemCount;
			auto_ptr<TextValidator> textValidator;
		};
	}
}

#endif
