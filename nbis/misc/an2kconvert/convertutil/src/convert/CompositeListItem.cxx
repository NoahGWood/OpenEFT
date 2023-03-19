#include "convert/CompositeListItem.hxx"

#include <iostream>

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	CompositeListItem::CompositeListItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, ItemStep itemStep, SubstrIndex substrIndex, Formatter* formatter)
	: CompositeItem(elementName, ns, fieldId, subfieldIndex, itemIndex, substrIndex, SubstrLen(INF)) {
		this->itemStep = itemStep.value;
	}

	CompositeListItem::CompositeListItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, ItemStep itemStep, SubstrIndex substrIndex, SubstrLen substrLen, Formatter* formatter)
	: CompositeItem(elementName, ns, fieldId, subfieldIndex, itemIndex, substrIndex, substrLen) {
		this->itemStep = itemStep.value;
	}

	void CompositeListItem::convert(list<XMLElement*>::const_iterator startElem, list<XMLElement*>::const_iterator endElem, part1::Record& part1Record) {
		while(startElem != endElem) {
			XMLElement const& elem = *(*startElem);
			CompositeItem::convert(elem, part1Record);
			startElem++;
			CompositeItem::incrementItem();
		}
	}

	auto_ptr<list<XMLElement*> > CompositeListItem::convert(part1::Record const& part1Record) {
		auto_ptr<list<XMLElement*> > elemList(new list<XMLElement*>());
		if(part1Record.hasField(fieldId)) {
			Field const& field = part1Record.getField(fieldId);
			while(field.subfieldsCount() > subfieldIndex && field.getSubfield(subfieldIndex).itemsCount() > itemIndex) {
				elemList->push_back(CompositeItem::convert(field).release());
				CompositeItem::incrementItem();
			}
		}
		return elemList;
	}
}
