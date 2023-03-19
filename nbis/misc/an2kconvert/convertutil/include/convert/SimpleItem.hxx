#ifndef SIMPLEITEM_HXX
#define SIMPLEITEM_HXX

#include "convert/CompositeItem.hxx"
#include "convert/format/Formatter.hxx"
#include "part1/FieldID.hxx"

#include <string>

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	/**
	 * This class is used to model XML elements whose value corresponds to a single item identified by
	 * its FieldID, subfieldIndex, and itemIndex.
	 */
	class SimpleItem : public CompositeItem {
	public:
		SimpleItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, Formatter* formatter = new Formatter);
		SimpleItem(string const& elementName, XMLNamespace ns, FieldID const& fieldId, SubfieldIndex subfieldIndex, ItemIndex itemIndex, ItemStep itemStep, Formatter* formatter = new Formatter);
	};
}

#endif
