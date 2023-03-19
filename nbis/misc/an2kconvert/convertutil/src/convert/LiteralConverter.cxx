#include "convert/LiteralConverter.hxx"

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	LiteralConverter::LiteralConverter(string const& elementName, XMLNamespace ns, string const& value)
	: Converter(elementName, ns), value(value) {}


	/**
	 * This method overrides the behavior of the convert() method in Converter.  It
	 * creates a new XMLElement containing a given value that has no counterpart in
	 * the binary representation of a record.
	 */
	auto_ptr<list<XMLElement*> > LiteralConverter::convert(part1::Record const& part1Record) {
		auto_ptr<list<XMLElement*> > elemList(new list<XMLElement*>());
		auto_ptr<XMLElement> elem(new XMLElement(elementId));
		elem->setText(value);
		elemList->push_back(elem.release());
		return elemList;
	}


	/**
	 * This method overrides the behavior of the convert() method in Converter.  It
	 * does nothing because a LiteralConverter represents a value that exists in the
	 * XML representation of a record, but not the binary representation.  Therefore,
	 * nothing needs to be done when this value is encountered in the XML.
	 */
	void LiteralConverter::convert(XMLElement const& elem, part1::Record& part1Record) {}
}
