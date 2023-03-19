#include "validate/ItemValidator.hxx"

namespace convert {
	namespace part1 {
		using namespace std;

		ItemValidator::ItemValidator(ItemCount itemCount, TextValidator* textValidator)
		: itemCount(itemCount.value),
		  textValidator(textValidator) {}

		size_t ItemValidator::getItemCount() const {
			return itemCount;
		}

		void ItemValidator::validateItem(Item const& item, Context& context, ValidationResult& valResult, ValidationLevel vl) const {
			textValidator->validateText(item.toString(), context, valResult, vl);
		}
	}
}
