#include "convert/Converter.hxx"
#include "utils.hxx"

namespace convert {
	using namespace std;
	using namespace convert::part1;
	using namespace convert::part2;

	Converter::Converter(string const& elementName, XMLNamespace ns, Formatter* formatter)
	: elementId(elementName, ns),
	  formatter(formatter) {}


	Converter::~Converter() {
		deleteContents<Converter*>(childNodes);
	}


	ElementID const& Converter::getElementId() const {
		return elementId;
	}


	void Converter::addChild(auto_ptr<Converter> node) {
		childNodes.push_back(node.release());
	}


	/**
	 * This method creates a new XMLElement and recursively creates its child elements.
	 * The new element is returned as the only element in the returned list.  If no child
	 * elements are created for the new XMLElement, the returned list will be empty.  This
	 * prevents the creation of empty XMLElements for optional elements that aren't present.
	 *
	 * part1Record - the Part1Record being converted
	 */
	auto_ptr<list<XMLElement*> > Converter::convert(part1::Record const& part1Record) {
		//create new XMLElement
		auto_ptr<XMLElement> elem(new XMLElement(elementId));
		for(list<Converter*>::const_iterator it = childNodes.begin(); it != childNodes.end(); it++) {
			//call convert for each child node and add the resulting child XMLElements
			//as children of the XMLElement for this node
			Converter& converter = *(*it);
			elem->addChildren(converter.convert(part1Record));
		}
		auto_ptr<list<XMLElement*> > elemList(new list<XMLElement*>());
		if(!elem->getChildren().empty()) {
			//don't put the new XMLElement on the list to be returned if it has no children
			//this prevents the creation of empty elements for optional elements that aren't present
			elemList->push_back(elem.release());
		}
		return elemList;
	}


	/**
	 * This method converts a series of XMLElements that have the same ElementID by calling
	 * convert(XMLElement, Part1Record).
	 *
	 * startElem - an iterator representing the first XMLElement to be converted
	 * endElem - an iterator representing the end of the list of XMLElements
	 * part1Record - the Part1Record being populated
	 */
	void Converter::convert(list<XMLElement*>::const_iterator startElem, list<XMLElement*>::const_iterator endElem, part1::Record& part1Record) {
		for(list<XMLElement*>::const_iterator it = startElem; it != endElem; it++) {
			//convert each instance of this element
			XMLElement const& elem = *(*it);
			convert(elem, part1Record);
		}
	}


	/**
	 * This method processes the child elements of elem by matching them to the child
	 * nodes of the Converter based on ElementID.
	 * If an element has missing child elements, all of the child elements that are
	 * present will convert properly.  However, if an element has an unexpected child
	 * element that element will be skipped.  If its child elements are not in the
	 * expected order, subsequent elements won't be processed properly, but no error
	 * will be generated.  Element presence and order should already have been validated.
	 *
	 * elem - the XMLElement to convert
	 * part1Record - the Part1Record being populated
	 */
	void Converter::convert(XMLElement const& elem, part1::Record& part1Record) {
		list<XMLElement*>::const_iterator elem_start = elem.getChildren().begin();
		list<XMLElement*>::const_iterator elem_end = elem.getChildren().begin();
		for(list<Converter*>::const_iterator it = childNodes.begin(); it != childNodes.end(); it++) {
			Converter& converter = *(*it);
			elem_start = elem_end;
			if(elem_end != elem.getChildren().end() && (*elem_end)->getElementId() != converter.getElementId()) {
				//if the next element in the list isn't the element we're looking for, we iterate down
				//the list until we find the element we're looking for or we run out of elements
				while(elem_end != elem.getChildren().end() && (*elem_end)->getElementId() != converter.getElementId()) {
					elem_end++;
				}
				if(elem_end == elem.getChildren().end()) {
					//if we ran out of elements without finding the element we were looking for,
					//we reset the position of elem_end back to where we started looking so that
					//the converter will be able to convert the empty set of elements later and
					//any remaining converters can process the part of the list we just checked
					elem_end = elem_start;
				} else {
					//if we found the element we were looking for, we reset elem_start to the
					//position of the first element that matches the converter (elem_end)
					elem_start = elem_end;
				}
			}
			while(elem_end != elem.getChildren().end() && (*elem_end)->getElementId() == converter.getElementId()) {
				//we iterate down the list until we run out of elements that match the converter
				//(there may be no elements that match the converter at this point)
				elem_end++;
			}
			//we convert the elements that we found that match the converter (if any)
			converter.convert(elem_start, elem_end, part1Record);
		}
	}


	/**
	 * This method propagates calls to incrementSubfield() to the child nodes of this Converter.
	 */
	void Converter::incrementSubfield() {
		//call incrementSubfield() on each child node
		for(list<Converter*>::const_iterator it = childNodes.begin(); it != childNodes.end(); it++) {
			Converter& converter = *(*it);
			converter.incrementSubfield();
		}
	}


	/**
	 * This method propagates calls to incrementItem() to the child nodes of this Converter.
	 */
	void Converter::incrementItem() {
		//call incrementItem() on each child node
		for(list<Converter*>::const_iterator it = childNodes.begin(); it != childNodes.end(); it++) {
			Converter& converter = *(*it);
			converter.incrementItem();
		}
	}
}
