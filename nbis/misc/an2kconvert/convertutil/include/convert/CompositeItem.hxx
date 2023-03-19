#ifndef COMPOSITEITEM_HXX
#define COMPOSITEITEM_HXX

#include "convert/Converter.hxx"
#include "convert/format/Formatter.hxx"
#include "part1/Field.hxx"
#include "part1/FieldID.hxx"
#include "part1/Record.hxx"
#include "part2/XMLElement.hxx"
#include "utils.hxx"

#include <list>
#include <memory>
#include <string>

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	class SubfieldIndex {
	public:
		SubfieldIndex(size_t value) : value(value) {};
		size_t value;
	};

	class ItemIndex {
	public:
		ItemIndex(size_t value) : value(value) {};
		size_t value;
	};

	class ItemStep {
	public:
		ItemStep(size_t value) : value(value) {};
		size_t value;
	};

	class SubstrIndex {
	public:
		SubstrIndex(size_t value) : value(value) {};
		size_t value;
	};

	class SubstrLen {
	public:
		SubstrLen(size_t value) : value(value) {};
		size_t value;
	};

	class CompositeItem : public Converter {
	public:
		CompositeItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, SubstrIndex substrIndex, Formatter* formatter = new Formatter);
		CompositeItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, SubstrIndex substrIndex, SubstrLen substrLen, Formatter* formatter = new Formatter);
		CompositeItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, ItemStep itemStep, SubstrIndex substrIndex, Formatter* formatter = new Formatter);
		CompositeItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, ItemStep itemStep, SubstrIndex substrIndex, SubstrLen substrLen, Formatter* formatter = new Formatter);
		virtual void convert(XMLElement const& elem, part1::Record& part1Record);
		virtual auto_ptr<list<XMLElement*> > convert(part1::Record const& part1Record);
		void incrementSubfield();
		void incrementItem();

	protected:
		virtual auto_ptr<XMLElement> convert(Field const& field);
		FieldID fieldId;
		size_t subfieldIndex;
		size_t itemIndex;
		size_t itemStartIndex;
		size_t itemStep;
		size_t substrIndex;
		size_t substrLen;
	};
}

#endif
