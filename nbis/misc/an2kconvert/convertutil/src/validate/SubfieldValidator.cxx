#include "validate/SubfieldValidator.hxx"
#include "validate/ValidationMessage.hxx"
#include "Errors.hxx"
#include "utils.hxx"

#include <iostream>
#include <sstream>

namespace convert {
	namespace part1 {
		using namespace std;

		SubfieldValidator::SubfieldValidator(SubfieldCount subfieldCount, ItemMin itemMin)
		: subfieldCount(subfieldCount.value),
		  itemMin1(itemMin.value1),
		  itemMin2(itemMin.value2),
		  itemMax1(INF),
		  itemMax2(INF) {}

		SubfieldValidator::SubfieldValidator(SubfieldCount subfieldCount, ItemMin itemMin, ItemMax itemMax)
		: subfieldCount(subfieldCount.value),
		  itemMin1(itemMin.value1),
		  itemMin2(itemMin.value2),
		  itemMax1(itemMax.value1),
		  itemMax2(itemMax.value2) {}

		SubfieldValidator::~SubfieldValidator() {
			deleteContents<ItemValidator*>(itemValidators);
		}

		size_t SubfieldValidator::getSubfieldCount() const {
			return subfieldCount;
		}

		void SubfieldValidator::addValidator(auto_ptr<ItemValidator> validator) {
			itemValidators.push_back(validator.release());
		}

		void SubfieldValidator::validateSubfield(Subfield const& subfield, Context& context, ValidationResult& valResult, ValidationLevel vl) const {
			validateItemCount(subfield, context, valResult);

			size_t itemIndex = 0;
			for(list<ItemValidator*>::const_iterator it = itemValidators.begin(); it != itemValidators.end(); it++) {
				ItemValidator const& val = *(*it);
				size_t itemCount = 0;
				while(itemIndex < subfield.itemsCount() && itemCount < val.getItemCount()) {
					context.setItemIndex(itemIndex + 1);
					val.validateItem(subfield.getItem(itemIndex), context, valResult, vl);
					context.unsetItemIndex();
					itemCount++;
					itemIndex++;
				}
			}
		}

		void SubfieldValidator::validateItemCount(Subfield const& subfield, Context& context, ValidationResult& valResult) const {
			if(subfield.itemsCount() < itemMin1) {
				//if itemsCount is less than both min values (min1 should be <= min2), throw an error
				stringstream ss;
				ss << "not enough items in subfield, min " << itemMin1 << ", got " << subfield.itemsCount();
				valResult.addError(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())));
				throw ValidationError();
			} else if(subfield.itemsCount() < itemMin2) {
				//if itemsCount is only less than one of the min values, give a warning
				stringstream ss;
				ss << "not enough items in subfield, min " << itemMin2 << ", got " << subfield.itemsCount();
				valResult.addWarning(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())));
			}

			if(subfield.itemsCount() > itemMax2) {
				//if itemsCount is greater than both max values (max2 should be >= max1), throw an error
				stringstream ss;
				ss << "too many items in subfield, max " << itemMax2 << ", got " << subfield.itemsCount();
				valResult.addError(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())));
				throw ValidationError();
			} else if(subfield.itemsCount() > itemMax1) {
				//if itemsCount is only greater than one of the max values, give a warning
				stringstream ss;
				ss << "too many items in subfield, max " << itemMax1 << ", got " << subfield.itemsCount();
				valResult.addWarning(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())));
			}
		}
	}
}
