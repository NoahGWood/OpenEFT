#ifndef XMLELEMENT_HXX
#define XMLELEMENT_HXX

#include "ElementID.hxx"

#include <xercesc/dom/DOM.hpp>

#include <list>
#include <memory>
#include <string>

namespace convert {
	namespace part2 {
		using namespace std;

		class XMLElement {
		public:
			XMLElement(ElementID const& elementId);
			XMLElement(xercesc::DOMNode const& node);
			ElementID const& getElementId() const;
			string const& getText() const;
			void setText(string const& text);
			list<XMLElement*> const& getChildren() const;
			XMLElement const& findChild(ElementID const& elementId) const;
			void addChild(auto_ptr<XMLElement> elem);
			void addChildren(auto_ptr<list<XMLElement*> > elems);
			auto_ptr<XMLElement> removeChild(size_t index);
			bool operator==(XMLElement const& elem) const;
			~XMLElement();

			static const XMLElement MISSING_ELEMENT;

		private:
			auto_ptr<string> xmlChToString(XMLCh const* xmlCh) const;
			auto_ptr<string> getElementText(xercesc::DOMText const& node) const;
			bool isWSNode(xercesc::DOMText const& node) const;

			ElementID elementId;
			string text;
			list<XMLElement*> childElements;
		};

	}
}

#endif
