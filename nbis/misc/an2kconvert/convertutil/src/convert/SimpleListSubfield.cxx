#include "convert/SimpleListSubfield.hxx"

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	SimpleListSubfield::SimpleListSubfield(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, Formatter* formatter)
	: CompositeItem(elementName, ns, fieldId, subfieldIndex, ItemIndex(0), SubstrIndex(0), formatter) {}

	SimpleListSubfield::SimpleListSubfield(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, Formatter* formatter)
	: CompositeItem(elementName, ns, fieldId, subfieldIndex, itemIndex, SubstrIndex(0), formatter) {}

	void SimpleListSubfield::convert(XMLElement const& elem, part1::Record& part1Record) {
		CompositeItem::convert(elem, part1Record);
		incrementSubfield();
	}

	auto_ptr<list<XMLElement*> > SimpleListSubfield::convert(part1::Record const& part1Record) {
		auto_ptr<list<XMLElement*> > elemList(new list<XMLElement*>());
		if(part1Record.hasField(fieldId)) {
			Field const& field = part1Record.getField(fieldId);
			while(field.subfieldsCount() > subfieldIndex && field.getSubfield(subfieldIndex).itemsCount() > itemIndex) {
				elemList->push_back(CompositeItem::convert(field).release());
				incrementSubfield();
			}
		}
		return elemList;
	}
}
