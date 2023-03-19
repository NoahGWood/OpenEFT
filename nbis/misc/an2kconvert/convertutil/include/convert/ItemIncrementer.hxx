#ifndef ITEMINCREMENTER_HXX
#define ITEMINCREMENTER_HXX

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
	 * the XML element represented by this Converter is encountered, the incrementItem()
	 * is called on its child nodes.
	 *
	 * An example of this relationship is
	 * <itl:FingerprintImageSegmentPositionPolygon>
	 *   <itl:PositionPolygonVertex>
	 *     <ansi-nist:PositionHorizontalCoordinateValue>
	 *     <ansi-nist:PositionVerticalCoordinateValue>
	 * in Type 14 records.
	 * PositionPolygonVertex is represented by an ItemIncrementer and its child elements are
	 * represented by SimpleItems.  The child elements correspond to a consecutive pair of
	 * items from the same subfield and there can be multiple occurrences of PositionPolygonVertex,
	 * each corresponding to another pair of consecutive items from the same subfield.
	 *
	 */
	class ItemIncrementer : public Converter {
	public:
		ItemIncrementer(string const& elementName, XMLNamespace ns);
		virtual auto_ptr<list<XMLElement*> > convert(part1::Record const& part1Record);
		virtual void convert(XMLElement const& elem, part1::Record& part1Record);
	};
}

#endif
