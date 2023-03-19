#include "validate/Context.hxx"

namespace convert {
	using namespace std;
	using namespace part1;
	using namespace part2;

	Context::Context()
	: fileType(AN_PART2),
	  fieldId(FieldID::MISSING_FIELD),
	  subfieldIndex(-1),
	  itemIndex(-1) {}

	Context::Context(FieldID const& fieldId)
	: fileType(AN_PART1),
	  fieldId(fieldId),
	  subfieldIndex(-1),
	  itemIndex(-1) {}

	FileType Context::getFileType() const {
		return fileType;
	}

	void Context::setSubfieldIndex(size_t index) {
		subfieldIndex = index;
	}

	void Context::unsetSubfieldIndex() {
		subfieldIndex = -1;
	}

	void Context::setItemIndex(size_t index) {
		itemIndex = index;
	}

	void Context::unsetItemIndex() {
		itemIndex = -1;
	}

	void Context::pushElementId(ElementID const& elementId) {
		elementIds.push_back(elementId);
	}

	void Context::popElementId() {
		elementIds.pop_back();
	}

	void Context::printContext(ostream& ostr) const {
		if(isPart1(fileType)) {
			printPart1Context(ostr);
		}
		if(isPart2(fileType)) {
			printPart2Context(ostr);
		}
	}

	void Context::printPart1Context(ostream& ostr) const {
		ostr << "Field " << fieldId.toString();
		if(subfieldIndex != -1) {
			ostr << ", Subfield " << subfieldIndex;
		}
		if(itemIndex != -1) {
			ostr << ", Item " << itemIndex;
		}
		ostr << endl;
	}

	void Context::printPart2Context(ostream& ostr) const {
		int tabs = 0;
		for(list<ElementID>::const_iterator it = elementIds.begin(); it != elementIds.end(); it++) {
			ElementID const& elemId = *it;
			ostr << "  ";
			for(int i = 0; i < tabs; i++) {
				ostr << "  ";
			}
			ostr << elemId.openElement() << endl;
			tabs++;
		}
	}
}
