#ifndef SUBFIELDINCREMENTER_HXX
#define SUBFIELDINCREMENTER_HXX

#include "convert/Converter.hxx"
#include "part1/Record.hxx"
#include "part2/XMLElement.hxx"

#include <list>
#include <memory>
#include <string>

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	/**
	 * This class is used to model basic XML elements that have child elements.  Each time
	 * the XML element represented by this Converter is encountered, the incrementSubfield()
	 * is called on its child nodes.
	 *
	 * An example of this relationship is
	 * <ansi-nist:ContentRecordSummary>
	 *   <ansi-nist:ImageReferenceIdentification>
	 *     <nc:IdentificationID>
	 *   <ansi-nist:RecordCategoryCode>
	 * in Type 1 records.
	 * ContentRecordSummary is represented by a SubfieldIncrementer and its child elements are
	 * SimpleItems.  The child elements correspond to two items in the same subfield, and each
	 * occurrence of ContentRecordSummary corresponds to a new subfield.
	 */
	class SubfieldIncrementer : public Converter {
	public:
		SubfieldIncrementer(string const& elementName, XMLNamespace ns);
		virtual auto_ptr<list<XMLElement*> > convert(part1::Record const& part1Record);
		virtual void convert(XMLElement const& elem, part1::Record& part1Record);
	};
}

#endif
