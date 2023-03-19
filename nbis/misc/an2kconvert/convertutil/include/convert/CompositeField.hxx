#ifndef COMPOSITEFIELD_HXX
#define COMPOSITEFIELD_HXX

#include "convert/CompositeSubfield.hxx"
#include "convert/format/Formatter.hxx"
#include "part1/FieldID.hxx"

#include <string>

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	class CompositeField : public CompositeSubfield {
	public:
		CompositeField(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubstrIndex substrIndex, Formatter* formatter = new Formatter);
		CompositeField(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubstrIndex substrIndex, SubstrLen substrLen, Formatter* formatter = new Formatter);
	};
}

#endif
