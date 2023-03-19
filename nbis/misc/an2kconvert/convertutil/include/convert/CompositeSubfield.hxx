#ifndef COMPOSITESUBFIELD_HXX
#define COMPOSITESUBFIELD_HXX

#include "convert/CompositeItem.hxx"
#include "convert/format/Formatter.hxx"
#include "part1/FieldID.hxx"

#include <string>

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	class CompositeSubfield : public CompositeItem {
	public:
		CompositeSubfield(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, SubstrIndex substrIndex, Formatter* formatter = new Formatter);
		CompositeSubfield(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, SubstrIndex substrIndex, SubstrLen substrLen, Formatter* formatter = new Formatter);
	};
}

#endif
