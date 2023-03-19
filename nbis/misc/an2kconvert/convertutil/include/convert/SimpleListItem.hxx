#ifndef SIMPLELISTITEM_HXX
#define SIMPLELISTITEM_HXX

#include "convert/CompositeListItem.hxx"

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	class SimpleListItem : public CompositeListItem {
	public:
		SimpleListItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, ItemStep itemStep, Formatter* formatter = new Formatter);
	};
}

#endif
