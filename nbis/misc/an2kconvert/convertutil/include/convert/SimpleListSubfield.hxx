#ifndef SIMPLELISTSUBFIELD_HXX
#define SIMPLELISTSUBFIELD_HXX

#include "convert/CompositeItem.hxx"

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	class SimpleListSubfield : public CompositeItem {
	public:
		SimpleListSubfield(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, Formatter* formatter = new Formatter);
		SimpleListSubfield(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, Formatter* formatter = new Formatter);
		void convert(XMLElement const& elem, part1::Record& part1Record);
		auto_ptr<list<XMLElement*> > convert(part1::Record const& part1Record);
	};
}

#endif
