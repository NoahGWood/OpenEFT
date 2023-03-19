#ifndef CONVERTER_HXX
#define CONVERTER_HXX

#include "convert/format/Formatter.hxx"
#include "part1/Record.hxx"
#include "part2/ElementID.hxx"
#include "part2/Record.hxx"
#include "part2/XMLElement.hxx"
#include "utils.hxx"

#include <list>
#include <memory>
#include <string>

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	/**
	 * This class is used to model basic XML elements that have child elements.  It also provides
	 * a base class for all other types of Converters.
	 */
	class Converter {
	public:
		Converter(string const& elementName, XMLNamespace ns, Formatter* formatter = new Formatter);
		~Converter();
		ElementID const& getElementId() const;
		void addChild(auto_ptr<Converter> node);
		virtual auto_ptr<list<XMLElement*> > convert(part1::Record const& part1Record);
		virtual void convert(list<XMLElement*>::const_iterator startElem, list<XMLElement*>::const_iterator endElem, part1::Record& part1Record);
		virtual void convert(XMLElement const& elem, part1::Record& part1Record);
		virtual void incrementSubfield();
		virtual void incrementItem();

	protected:
		ElementID elementId;
		list<Converter*> childNodes;
		auto_ptr<Formatter> formatter;
	};
}

#endif
