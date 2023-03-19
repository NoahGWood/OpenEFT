#include "convert/SimpleListItem.hxx"

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	SimpleListItem::SimpleListItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, ItemStep itemStep, Formatter* formatter)
	: CompositeListItem(elementName, ns, fieldId, subfieldIndex, itemIndex, itemStep, SubstrIndex(0)) {}
}
