#ifndef LITERALCONVERTER_HXX
#define LITERALCONVERTER_HXX

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
	 * This class is used to model XML elements that have no counterpart in the Part 1
	 * representation of a record.  The primary example is the <ansi-nist:RecordCategoryCode>
	 * element that is present in every record.
	 */
	class LiteralConverter : public Converter {
	public:
		LiteralConverter(string const& elementName, XMLNamespace ns, string const& value);
		virtual auto_ptr<list<XMLElement*> > convert(part1::Record const& part1Record);
		virtual void convert(XMLElement const& elem, part1::Record& part1Record);

	private:
		string value;
	};
}

#endif
