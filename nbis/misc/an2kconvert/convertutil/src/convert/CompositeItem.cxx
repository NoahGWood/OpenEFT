#include "convert/CompositeItem.hxx"

#include <iostream>

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	CompositeItem::CompositeItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, SubstrIndex substrIndex, Formatter* formatter)
	: Converter(elementName, ns, formatter),
	  fieldId(fieldId),
	  subfieldIndex(subfieldIndex.value),
	  itemIndex(itemIndex.value),
	  itemStartIndex(itemIndex.value),
	  substrIndex(substrIndex.value),
	  substrLen(INF),
	  itemStep(0) {}

	CompositeItem::CompositeItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, SubstrIndex substrIndex, SubstrLen substrLen, Formatter* formatter)
	: Converter(elementName, ns, formatter),
	  fieldId(fieldId),
	  subfieldIndex(subfieldIndex.value),
	  itemIndex(itemIndex.value),
	  itemStartIndex(itemIndex.value),
	  substrIndex(substrIndex.value),
	  substrLen(substrLen.value),
	  itemStep(0) {}

	CompositeItem::CompositeItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, ItemStep itemStep, SubstrIndex substrIndex, Formatter* formatter)
	: Converter(elementName, ns, formatter),
	  fieldId(fieldId),
	  subfieldIndex(subfieldIndex.value),
	  itemIndex(itemIndex.value),
	  itemStartIndex(itemIndex.value),
	  substrIndex(substrIndex.value),
	  substrLen(INF),
	  itemStep(itemStep.value) {}

	CompositeItem::CompositeItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, ItemStep itemStep, SubstrIndex substrIndex, SubstrLen substrLen, Formatter* formatter)
	: Converter(elementName, ns, formatter),
	  fieldId(fieldId),
	  subfieldIndex(subfieldIndex.value),
	  itemIndex(itemIndex.value),
	  itemStartIndex(itemIndex.value),
	  substrIndex(substrIndex.value),
	  substrLen(substrLen.value),
	  itemStep(itemStep.value) {}

	void CompositeItem::convert(XMLElement const& elem, part1::Record& part1Record) {
		if(!part1Record.hasField(fieldId)) {
			part1Record.addField(auto_ptr<Field>(new Field(fieldId)));
		}
		Field& field = part1Record.getField(fieldId);
		field.appendItem(formatter->formatPart1(elem.getText()), subfieldIndex, itemIndex);
	}

	auto_ptr<list<XMLElement*> > CompositeItem::convert(part1::Record const& part1Record) {
		auto_ptr<list<XMLElement*> > elemList(new list<XMLElement*>());
		if(part1Record.hasField(fieldId)) {
			Field const& field = part1Record.getField(fieldId);
			if(field.subfieldsCount() > subfieldIndex && field.getSubfield(subfieldIndex).itemsCount() > itemIndex) {
				elemList->push_back(convert(field).release());
			}
		}
		return elemList;
	}

	auto_ptr<XMLElement> CompositeItem::convert(Field const& field) {
		string const& text = field.getSubfield(subfieldIndex).getItem(itemIndex).toString();
		auto_ptr<XMLElement> elem(new XMLElement(elementId));
		elem->setText(formatter->formatPart2(text.substr(substrIndex, substrLen)));
		return elem;
	}

	void CompositeItem::incrementSubfield() {
		subfieldIndex++;
		itemIndex = itemStartIndex;
	}

	void CompositeItem::incrementItem() {
		itemIndex += itemStep;
	}
}
