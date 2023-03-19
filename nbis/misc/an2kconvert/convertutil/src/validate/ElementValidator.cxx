#include "validate/ElementValidator.hxx"
#include "Errors.hxx"
#include "utils.hxx"

#include <iostream>
#include <sstream>

namespace convert {
	namespace part2 {
		using namespace std;

		ElementValidator::ElementValidator(string const& elementName, XMLNamespace ns, ElemType elemType, TextValidator* textValidator)
		: elementId(elementName, ns),
		  textValidator(textValidator) {
			switch(elemType) {
			case SingleMandatory:
				min1 = min2 = 1;
				max1 = max2 = 1;
				break;
			case SingleOptional:
				min1 = min2 = 0;
				max1 = max2 = 1;
				break;
			case UnlimitedMandatory:
				min1 = min2 = 1;
				max1 = max2 = INF;
				break;
			case UnlimitedOptional:
				min1 = min2 = 0;
				max1 = max2 = INF;
				break;
			}
		}

		ElementValidator::ElementValidator(string const& elementName, XMLNamespace ns, ElemMin min, TextValidator* textValidator)
		: elementId(elementName, ns),
		  min1(min.value1),
		  min2(min.value2),
		  max1(INF),
		  max2(INF),
		  textValidator(textValidator) {}

		ElementValidator::ElementValidator(string const& elementName, XMLNamespace ns, ElemMin min, ElemMax max, TextValidator* textValidator)
		: elementId(elementName, ns),
		  min1(min.value1),
		  min2(min.value2),
		  max1(max.value1),
		  max2(max.value2),
		  textValidator(textValidator) {}

		ElementValidator::~ElementValidator() {
			deleteContents<ElementValidator*>(childNodes);
		}

		ElementID const& ElementValidator::getElementId() const {
			return elementId;
		}

		void ElementValidator::addValidator(auto_ptr<ElementValidator> validator) {
			childNodes.push_back(validator.release());
		}

		void ElementValidator::validateElement(XMLElement const& elem, Context& context, ValidationResult& result, ValidationLevel vl) const {
			context.pushElementId(getElementId());
			if(elem.getElementId() != getElementId()) {
				stringstream ss;
				ss << "invalid element " << elem.getElementId().openElement();
				result.addError(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())));
				throw ValidationError();
			}
			validateElementText(elem, context, result, vl);
			validateElementChildren(elem, context, result, vl);
			context.popElementId();
		}

		void ElementValidator::validateElement(list<XMLElement*>::const_iterator const& startElem, list<XMLElement*>::const_iterator const& endElem, Context& context, ValidationResult& result, ValidationLevel vl) const {
			context.pushElementId(getElementId());
			size_t elemCount = 0;
			for(list<XMLElement*>::const_iterator it = startElem; it != endElem; it++) {
				elemCount++;
			}
			validateElementCount(elemCount, context, result);

			for(list<XMLElement*>::const_iterator it = startElem; it != endElem; it++) {
				XMLElement const& elem = *(*it);
				validateElementText(elem, context, result, vl);
				validateElementChildren(elem, context, result, vl);
			}
			context.popElementId();
		}

		void ElementValidator::validateElementChildren(XMLElement const& elem, Context& context, ValidationResult& result, ValidationLevel vl) const {
			list<XMLElement*>::const_iterator elem_start = elem.getChildren().begin();
			list<XMLElement*>::const_iterator elem_end = elem.getChildren().begin();
			for(list<ElementValidator*>::const_iterator it = childNodes.begin(); it != childNodes.end(); it++) {
				ElementValidator const& validator = *(*it);
				elem_start = elem_end;
				if(elem_end != elem.getChildren().end() && (*elem_end)->getElementId() != validator.getElementId()) {
					//if the next element in the list isn't the element we're looking for, we iterate down
					//the list until we find the element we're looking for or we run out of elements
					list<ValidationMessage*> skippedElements;
					while(elem_end != elem.getChildren().end() && (*elem_end)->getElementId() != validator.getElementId()) {
						stringstream ss;
						ss << "skipping element " << (*elem_end)->getElementId().openElement();
						skippedElements.push_back(new ValidationMessage(context, ss.str()));
						elem_end++;
					}
					if(elem_end == elem.getChildren().end()) {
						//if we ran out of elements without finding the element we were looking for,
						//we reset the position of elem_end back to where we started looking so that
						//the validator will be able to validate the empty set of elements later and
						//any remaining validators can process the part of the list we just checked
						deleteContents<ValidationMessage*>(skippedElements);
						elem_end = elem_start;
					} else {
						//if we found the element we were looking for, we create warnings for the
						//elements we skipped, and reset elem_start to the position of the first element
						//that matches the validator (elem_end)
						while(!skippedElements.empty()) {
							ValidationMessage* m = skippedElements.front();
							skippedElements.pop_front();
							result.addWarning(auto_ptr<ValidationMessage>(m));
						}
						elem_start = elem_end;
					}
				}
				while(elem_end != elem.getChildren().end() && (*elem_end)->getElementId() == validator.getElementId()) {
					//we iterate down the list until we run out of elements that match the validator
					//(there may be no elements that match the validator at this point)
					elem_end++;
				}
				//we validate the elements that we found that match the validator (if any)
				validator.validateElement(elem_start, elem_end, context, result, vl);
			}

			while(elem_end != elem.getChildren().end()) {
				//if there are unprocessed elements remaining in the list after all of the validators
				//have been processed, create a warning for each one
				stringstream ss;
				ss << "skipping element " << (*elem_end)->getElementId().openElement();
				result.addWarning(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())));
				elem_end++;
			}
		}

		void ElementValidator::validateElementText(XMLElement const& elem, Context& context, ValidationResult& result, ValidationLevel vl) const {
			textValidator->validateText(elem.getText(), context, result, vl);
		}

		void ElementValidator::validateElementCount(size_t elemCount, Context& context, ValidationResult& result) const {
			if(elemCount < min1) {
				//if elemCount is less than both min values (min1 should be <= min2), throw an error
				stringstream ss;
				ss << "not enough occurrences of element, min " << min1 << ", got " << elemCount;
				result.addError(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())));
				throw ValidationError();
			} else if(elemCount < min2) {
				//if elemCount is only less than one of the min values, give a warning
				stringstream ss;
				ss << "not enough occurrences of element, min " << min2 << ", got " << elemCount;
				result.addWarning(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())));
			}

			if(elemCount > max2) {
				//if elemCount is greater than both max values (max2 should be >= max1), throw an error
				stringstream ss;
				ss << "too many occurrences of element, max " << max2 << ", got " << elemCount;
				result.addError(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())));
				throw ValidationError();
			} else if(elemCount > max1) {
				//if elemCount is only greater than one of the max values, give a warning
				stringstream ss;
				ss << "too many occurrences of element, max " << max1 << ", got " << elemCount;
				result.addWarning(auto_ptr<ValidationMessage>(new ValidationMessage(context, ss.str())));
			}
		}
	}
}
