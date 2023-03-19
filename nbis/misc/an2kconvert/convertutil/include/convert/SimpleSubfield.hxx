#ifndef SIMPLESUBFIELD_HXX
#define SIMPLESUBFIELD_HXX

#include "convert/SimpleItem.hxx"
#include "convert/format/Formatter.hxx"
#include "part1/FieldID.hxx"

#include <string>

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	/**
	 * This class is used to model XML elements whose value corresponds to a single item in a
	 * single subfield identified by its FieldID and subfieldIndex.
	 */
	class SimpleSubfield : public SimpleItem {
	public:
		SimpleSubfield(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, Formatter* formatter = new Formatter);
	};
}

#endif
