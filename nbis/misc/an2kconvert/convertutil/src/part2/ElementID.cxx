#include "part2/ElementID.hxx"

#include <sstream>

namespace convert {
	namespace part2 {
		using namespace std;

		ElementID::ElementID(string const& elementName, XMLNamespace ns)
		: elementName(elementName),
		  ns(ns) {}

		ElementID::ElementID(string const& elementName, string const& nsPrefix)
		: elementName(elementName),
		  ns(stringToNamespace(nsPrefix)) {}

		string const& ElementID::getElementName() const {
			return elementName;
		}

		XMLNamespace ElementID::getNamespace() const {
			return ns;
		}

		string ElementID::openElement() const {
			stringstream ss;
			ss << "<" << namespaceToString(ns) << ":" << elementName << ">";
			return ss.str();
		}

		string ElementID::closeElement() const {
			stringstream ss;
			ss << "</" << namespaceToString(ns) << ":" << elementName << ">";
			return ss.str();
		}

		bool ElementID::operator==(ElementID const& elementId) const {
			return this->elementName == elementId.getElementName() && this->ns == elementId.getNamespace();
		}

		bool ElementID::operator!=(ElementID const& elementId) const {
			return !(*this == elementId);
		}

		XMLNamespace ElementID::stringToNamespace(string const& nsStr) const {
			if(nsStr == "ansi-nist") {
				return AN;
			}
			if(nsStr == "itl") {
				return ITL;
			}
			if(nsStr == "nc") {
				return NC;
			}
			if(nsStr == "incits") {
				return INCITS;
			}
			if(nsStr == "ebts") {
				return EBTS;
			}
			if(nsStr == "j") {
				return J;
			}
		}

		string ElementID::namespaceToString(XMLNamespace ns) const {
			switch(ns) {
			case AN:
				return "ansi-nist";
			case ITL:
				return "itl";
			case NC:
				return "nc";
			case INCITS:
				return "incits";
			case EBTS:
				return "ebts";
			case J:
				return "j";
			}
		}
	}
}
