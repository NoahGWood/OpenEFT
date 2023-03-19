#include "part2/XMLElement.hxx"
#include "Errors.hxx"
#include "utils.hxx"

#include <boost/regex.hpp>
#include <xercesc/util/XMLString.hpp>

namespace convert {
	namespace part2 {
		using namespace std;
		using namespace xercesc;

		XMLElement::XMLElement(ElementID const& elementId)
		: elementId(elementId) {}

		XMLElement::XMLElement(DOMNode const& node)
		: elementId(*(xmlChToString(node.getLocalName())), *(xmlChToString(node.getPrefix()))) {
			for(XMLSize_t i = 0; i < node.getChildNodes()->getLength(); i++) {
				DOMNode* currentNode = node.getChildNodes()->item(i);
				if(currentNode->getNodeType() == DOMNode::ELEMENT_NODE) {
					childElements.push_back(new XMLElement(*currentNode));
				} else if(currentNode->getNodeType() == DOMNode::TEXT_NODE) {
					DOMText const& textNode = *(dynamic_cast<DOMText*>(currentNode));
					if(!isWSNode(textNode)) {
						if(!text.empty()) {
							throw ParseError("Multiple non-whitespace text nodes");
							//there should only be one non-whitespace text node
						}
						text = *(getElementText(textNode));
					}
				} else {
					throw ParseError("Invalid XML node type");
					//there shouldn't be any nodes of other types
				}
			}
		}

		ElementID const& XMLElement::getElementId() const {
			return elementId;
		}

		string const& XMLElement::getText() const {
			return text;
		}

		void XMLElement::setText(string const& text) {
			this->text = text;
		}

		list<XMLElement*> const& XMLElement::getChildren() const {
			return childElements;
		}

		XMLElement const& XMLElement::findChild(ElementID const& elementId) const {
			for(list<XMLElement*>::const_iterator it = childElements.begin(); it != childElements.end(); it++) {
				XMLElement const& elem = *(*it);
				if(elem.getElementId() == elementId) {
					return elem;
				}
			}
			return MISSING_ELEMENT;
		}

		void XMLElement::addChild(auto_ptr<XMLElement> elem) {
			childElements.push_back(elem.release());
		}

		void XMLElement::addChildren(auto_ptr<list<XMLElement*> > elems) {
			childElements.splice(childElements.end(), *elems);
		}

		auto_ptr<XMLElement> XMLElement::removeChild(size_t index) {
			if(index >= childElements.size()) {
				throw logic_error("XMLElement.removeChild(): invalid index");
			}
			list<XMLElement*>::iterator it = childElements.begin();
			while(index > 0) {
				it++;
				index--;
			}
			auto_ptr<XMLElement> element(*it);
			childElements.erase(it);
			return element;
		}

		bool XMLElement::operator==(XMLElement const& elem) const {
			return this->getElementId() == elem.getElementId();
		}

		XMLElement::~XMLElement() {
			deleteContents<XMLElement*>(childElements);
		}

		const XMLElement XMLElement::MISSING_ELEMENT(ElementID("", ""));

		auto_ptr<string> XMLElement::xmlChToString(XMLCh const* xmlCh) const {
			char* cstr = XMLString::transcode(xmlCh);
			auto_ptr<string> str(new string(cstr));
			XMLString::release(&cstr);
			return str;
		}

		auto_ptr<string> XMLElement::getElementText(DOMText const& node) const {
			auto_ptr<string> text(xmlChToString(node.getData()));
			while(text->length() < node.getLength()) {
				*text += *(xmlChToString(node.substringData(text->length(), node.getLength())));
			}
			return text;
		}

		bool XMLElement::isWSNode(DOMText const& node) const {
			auto_ptr<string> text = getElementText(node);
			if(text->length() == 0) {
				return true;
			}

			string::const_iterator start = text->begin();
			string::const_iterator end = text->end();
			boost::match_results<string::const_iterator> results;
			boost::regex re("\\A\\s+\\z");
			boost::match_flag_type flags = boost::match_default;
			if(regex_search(start, end, results, re, flags)) {
				return true;
			} else {
				return false;
			}
		}
	}
}
