#include "convert/SimpleSubfield.hxx"

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	SimpleSubfield::SimpleSubfield(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, Formatter* formatter)
	: SimpleItem(elementName, ns, fieldId, subfieldIndex, ItemIndex(0), formatter) {}
}
