#include "convert/CompositeSubfield.hxx"

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	CompositeSubfield::CompositeSubfield(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, SubstrIndex substrIndex, Formatter* formatter)
	: CompositeItem(elementName, ns, fieldId, subfieldIndex, ItemIndex(0), substrIndex, formatter) {}

	CompositeSubfield::CompositeSubfield(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, SubstrIndex substrIndex, SubstrLen substrLen, Formatter* formatter)
	: CompositeItem(elementName, ns, fieldId, subfieldIndex, ItemIndex(0), substrIndex, substrLen, formatter) {}
}
