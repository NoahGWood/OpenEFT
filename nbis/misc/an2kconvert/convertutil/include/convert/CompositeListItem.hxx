#ifndef COMPOSITELISTITEM_HXX
#define COMPOSITELISTITEM_HXX

#include "convert/CompositeItem.hxx"

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	class CompositeListItem : public CompositeItem {
	public:
		CompositeListItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, ItemStep itemStep, SubstrIndex substrIndex, Formatter* formatter = new Formatter);
		CompositeListItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, ItemStep itemStep, SubstrIndex substrIndex, SubstrLen substrLen, Formatter* formatter = new Formatter);
		void convert(list<XMLElement*>::const_iterator startElem, list<XMLElement*>::const_iterator endElem, part1::Record& part1Record);
		auto_ptr<list<XMLElement*> > convert(part1::Record const& part1Record);
	};
}

#endif
