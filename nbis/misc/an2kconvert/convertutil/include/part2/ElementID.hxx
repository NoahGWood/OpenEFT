#ifndef ELEMENTID_HXX
#define ELEMENTID_HXX

#include <string>

namespace convert {
	namespace part2 {
		using namespace std;

		enum XMLNamespace {
			AN,
			ITL,
			NC,
			INCITS,
			EBTS,
			J
		};

		class ElementID {
		public:
			ElementID(string const& elementName, XMLNamespace ns);
			ElementID(string const& elementName, string const& nsPrefix);
			string const& getElementName() const;
			XMLNamespace getNamespace() const;
			string openElement() const;
			string closeElement() const;
			bool operator==(ElementID const& elementId) const;
			bool operator!=(ElementID const& elementId) const;

		private:
			XMLNamespace stringToNamespace(string const& nsStr) const;
			string namespaceToString(XMLNamespace ns) const;
			string elementName;
			XMLNamespace ns;
		};
	}
}

#endif
