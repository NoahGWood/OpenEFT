#ifndef ELEMENTVALIDATOR_HXX
#define ELEMENTVALIDATOR_HXX

#include "validate/text/TextValidator.hxx"
#include "part2/ElementID.hxx"
#include "part2/XMLElement.hxx"
#include "validate/Context.hxx"
#include "validate/ValidationResult.hxx"
#include "validate/Context.hxx"
#include "utils.hxx"

#include <list>
#include <memory>
#include <string>

namespace convert {
	namespace part2 {

	using namespace std;

		class ElemMin {
		public:
			ElemMin(size_t value) : value1(value), value2(value) {};
			ElemMin(size_t value1, size_t value2) : value1(value1), value2(value2) {};
			size_t value1;
			size_t value2;
		};

		class ElemMax {
		public:
			ElemMax(size_t value) : value1(value), value2(value) {};
			ElemMax(size_t value1, size_t value2) : value1(value1), value2(value2) {};
			size_t value1;
			size_t value2;
		};

		enum ElemType {
			SingleMandatory,
			SingleOptional,
			UnlimitedMandatory,
			UnlimitedOptional
		};

		class ElementValidator {
		public:
			ElementValidator(string const& elementName, XMLNamespace ns, ElemType elemType, TextValidator* textValidator = new TextValidator);
			ElementValidator(string const& elementName, XMLNamespace ns, ElemMin min, TextValidator* textValidator = new TextValidator);
			ElementValidator(string const& elementName, XMLNamespace ns, ElemMin min, ElemMax max, TextValidator* textValidator = new TextValidator);
			~ElementValidator();
			ElementID const& getElementId() const;
			void addValidator(auto_ptr<ElementValidator> validator);
			void validateElement(XMLElement const& elem, Context& context, ValidationResult& valResult, ValidationLevel vl) const;
			void validateElement(list<XMLElement*>::const_iterator const& startElem, list<XMLElement*>::const_iterator const& endElem, Context& context, ValidationResult& valResult, ValidationLevel vl) const;

		private:
			void validateElementChildren(XMLElement const& elem, Context& context, ValidationResult& valResult, ValidationLevel vl) const;
			void validateElementText(XMLElement const& elem, Context& context, ValidationResult& valResult, ValidationLevel vl) const;
			void validateElementCount(size_t elemCount, Context& context, ValidationResult& valResult) const;

			ElementID elementId;
			size_t min1;
			size_t min2;
			size_t max1;
			size_t max2;
			auto_ptr<TextValidator> textValidator;
			list<ElementValidator*> childNodes;
		};
	}
}

#endif
