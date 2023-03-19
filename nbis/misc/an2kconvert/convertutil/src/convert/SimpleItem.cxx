#include "convert/SimpleItem.hxx"

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	/**
	 * This constructor is used for SimpleItems that only have one occurrence per subfield.
	 */
	SimpleItem::SimpleItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, Formatter* formatter)
	: CompositeItem(elementName, ns, fieldId, subfieldIndex, itemIndex, SubstrIndex(0), formatter) {}


	/**
	 * This constructor is used for SimpleItems that can have multiple occurrences per subfield.
	 */
	SimpleItem::SimpleItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, ItemStep itemStep, Formatter* formatter)
	: CompositeItem(elementName, ns, fieldId, subfieldIndex, itemIndex, SubstrIndex(0), formatter) {
		this->itemStep = itemStep.value;
	}
}
