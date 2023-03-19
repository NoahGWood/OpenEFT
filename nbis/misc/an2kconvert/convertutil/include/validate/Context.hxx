#ifndef CONTEXT_HXX
#define CONTEXT_HXX

#include "ITLPackage.hxx"
#include "part1/FieldID.hxx"
#include "part2/ElementID.hxx"

#include <list>
#include <ostream>

namespace convert {
	using namespace std;
	using namespace part1;
	using namespace part2;

	class Context {
	public:
		Context();
		Context(FieldID const& fieldId);
		FileType getFileType() const;
		void setSubfieldIndex(size_t index);
		void unsetSubfieldIndex();
		void setItemIndex(size_t index);
		void unsetItemIndex();
		void pushElementId(ElementID const& elementId);
		void popElementId();
		void printContext(ostream& ostr) const;

	private:
		void printPart1Context(ostream& ostr) const;
		void printPart2Context(ostream& ostr) const;
		FileType fileType;
		list<ElementID> elementIds;
		FieldID fieldId;
		size_t subfieldIndex;
		size_t itemIndex;
	};
}

#endif
