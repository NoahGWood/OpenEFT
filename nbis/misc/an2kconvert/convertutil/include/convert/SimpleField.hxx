#ifndef SIMPLEFIELD_HXX
#define SIMPLEFIELD_HXX

#include "convert/SimpleSubfield.hxx"
#include "convert/format/Formatter.hxx"
#include "part1/FieldID.hxx"

#include <memory>
#include <string>

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	/**
	 * This class is used to model XML elements whose value corresponds to a field containing a single
	 * subfield with a single item.
	 */
	class SimpleField : public SimpleSubfield {
	public:
		SimpleField(string const& elementName, XMLNamespace ns, FieldID const& fieldId, Formatter* formatter = new Formatter);
	};
}

#endif

