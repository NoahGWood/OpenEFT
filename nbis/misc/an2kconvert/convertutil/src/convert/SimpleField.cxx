#include "convert/SimpleField.hxx"

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	SimpleField::SimpleField(string const& elementName, XMLNamespace ns, FieldID const& fieldId, Formatter* formatter)
	: SimpleSubfield(elementName, ns, fieldId, SubfieldIndex(0), formatter) {}
}
