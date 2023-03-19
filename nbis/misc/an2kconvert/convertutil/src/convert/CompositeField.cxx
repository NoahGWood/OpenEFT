#include "convert/CompositeField.hxx"

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	CompositeField::CompositeField(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubstrIndex substrIndex, Formatter* formatter)
	: CompositeSubfield(elementName, ns, fieldId, SubfieldIndex(0), substrIndex, formatter) {}

	CompositeField::CompositeField(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubstrIndex substrIndex, SubstrLen substrLen, Formatter* formatter)
	: CompositeSubfield(elementName, ns, fieldId, SubfieldIndex(0), substrIndex, substrLen, formatter) {}
}
