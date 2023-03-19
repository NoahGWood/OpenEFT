#include "convert/ItemIncrementer.hxx"

#include <iostream>

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	ItemIncrementer::ItemIncrementer(string const& elementName, XMLNamespace ns)
	: Converter(elementName, ns) {}


	/**
	 * This method overrides the behavior of the convert() method in Converter.  It
	 * calls the convert() method in the base class and then calls incrementItem().
	 * It repeats this until calling the convert() in the base class stops returning
	 * new XMLElements.  It then returns a list of all XMLElements generated.
	 */
	auto_ptr<list<XMLElement*> > ItemIncrementer::convert(part1::Record const& part1Record) {
		auto_ptr<list<XMLElement*> > elemList(new list<XMLElement*>);
		while(true) {
			//keep calling convert() and accumulating XMLElements until convert()
			//runs out of items and returns an empty list
			auto_ptr<list<XMLElement*> > newList(Converter::convert(part1Record));
			if(newList->empty()) {
				break;
			} else {
				elemList->splice(elemList->end(), *newList);
				incrementItem();
			}
		}
		return elemList;
	}


	/**
	 * This method overrides the behavior of the convert() method in Converter.  It
	 * calls incrementItem() after each instance of the element is processed.
	 */
	void ItemIncrementer::convert(XMLElement const& elem, part1::Record& part1Record) {
		Converter::convert(elem, part1Record);
		//call incrementItem() after processing each instance of this element
		incrementItem();
	}
}
