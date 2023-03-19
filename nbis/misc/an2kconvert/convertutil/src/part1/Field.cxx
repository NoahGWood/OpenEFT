#include "part1/Field.hxx"

namespace convert {
	namespace part1 {
		using namespace std;

		/**
		 * Constructs a new empty Field.
		 */
		Field::Field(FieldID const& fieldId)
		: fieldId(fieldId) {}


		/**
		 * Constructs a new Field with a single Subfield.
		 *
		 * subfield: Subfield to add to the Field.
		 */
		Field::Field(FieldID const& fieldId, auto_ptr<Subfield> subfield)
		: fieldId(fieldId) {
			addSubfield(subfield);
		}


		/**
		 * Constructs a new Field with a single Subfield with a single Item.
		 *
		 * item: Item to add to the Subfield.
		 */
		Field::Field(FieldID const& fieldId, auto_ptr<Item> item)
		: fieldId(fieldId) {
			addSubfield(item);
		}


		/**
		 * Constructs a new Field with a single Subfield with a single Item containing
		 * itemValue.
		 *
		 * itemValue: string containing the value of the item to be created in the
		 * Subfield.
		 */
		Field::Field(FieldID const& fieldId, string const& itemValue)
		: fieldId(fieldId) {
			addSubfield(itemValue);
		}


		/**
		 * Constructs a new Field from Part 1 binary data.  Data is parsed according
		 * to the specified FieldType.
		 *
		 * bytes: string containing Part 1 binary data.
		 * fieldType: FieldType to be used when parsing the data.
		 */
		Field::Field(FieldID const& fieldId, string const& bytes, FieldType fieldType)
		: fieldId(fieldId) {
			switch(fieldType) {
			case BinaryImageField:
				decodeBinaryImageField(bytes);
				break;
			case BinaryU8Field:
				decodeBinaryU8Field(bytes);
				break;
			case BinaryU16Field:
				decodeBinaryU16Field(bytes);
				break;
			case BinaryU32Field:
				decodeBinaryU32Field(bytes);
				break;
			case BinaryVectorField:
				decodeBinaryVectorField(bytes);
				break;
			case TaggedAsciiField:
				decodeTaggedAsciiField(bytes);
				break;
			case TaggedImageField:
				decodeTaggedImageField(bytes);
				break;
			}
		}


		/**
		 * Copy constructor.
		 */
		Field::Field(Field const& field)
		: fieldId(field.getFieldID() ) {
			//Don't copy Fields with NULL Subfields
			for(int i = 0; i < field.subfieldsCount(); i++) {
				addSubfield(auto_ptr<Subfield>(new Subfield(field.getSubfield(i))));
			}
		}


		/**
		 * Returns the length of the Field in bytes.  The returned length is equal to
		 * the size of the string returned by toBytesForFile(fieldType).  It is an
		 * error if the Field contains NULL Subfields.
		 *
		 * fieldType: FieldType to be used when determining the length of the Field.
		 */
		size_t Field::getLength(FieldType fieldType) const {
			if(hasMissingSubfields()) {
				throw logic_error("Field.getLength(): Field has missing Subfields");
			}
			size_t length = 0;
			SubfieldType subfieldType;
			switch(fieldType) {
			case BinaryImageField:
				subfieldType = BinaryImageSubfield;
				break;
			case BinaryU8Field:
				subfieldType = BinaryU8Subfield;
				break;
			case BinaryU16Field:
				subfieldType = BinaryU16Subfield;
				break;
			case BinaryU32Field:
				subfieldType = BinaryU32Subfield;
				break;
			case BinaryVectorField:
				subfieldType = BinaryVectorSubfield;
				break;
			case TaggedAsciiField:
				length += fieldId.toBytesForFile().size();
				subfieldType = TaggedAsciiSubfield;
				break;
			case TaggedImageField:
				length += fieldId.toBytesForFile().size();
				subfieldType = TaggedImageSubfield;
				break;
			}

			for(vector<Subfield*>::const_iterator it = subfields.begin(); it != subfields.end(); it++) {
				Subfield const& subfield = *(*it);
				length += subfield.getLength(subfieldType);
			}
			return length;
		}


		/**
		 * Returns the FieldID associated with the Field.
		 */
		FieldID const& Field::getFieldID() const {
			return fieldId;
		}


		/**
		 * Returns a string containing the Part 1 binary data representing the Field.
		 * The data will be formatted according the specified FieldType.  It is an
		 * error if the Field contains NULL Subfields.
		 *
		 * fieldType: the FieldType to be used to format the Field data.
		 */
		auto_ptr<string> Field::toBytesForFile(FieldType fieldType) const {
			if(hasMissingSubfields()) {
				throw logic_error("Field.toBytesForFile(): Field has missing Subfields");
			}
			switch(fieldType) {
			case BinaryImageField:
				return encodeBinaryImageField();
			case BinaryU8Field:
				return encodeBinaryU8Field();
			case BinaryU16Field:
				return encodeBinaryU16Field();
			case BinaryU32Field:
				return encodeBinaryU32Field();
			case BinaryVectorField:
				return encodeBinaryVectorField();
			case TaggedAsciiField:
				return encodeTaggedAsciiField();
			case TaggedImageField:
				return encodeTaggedImageField();
			}
		}


		/**
		 * Returns the Subfield in the Field at index if it exists.  It is an error
		 * if the index is invalid or it contains a NULL Subfield.  The returned Subfield
		 * is const.
		 *
		 * index: index of the Subfield to be returned.
		 */
		Subfield const& Field::getSubfield(size_t index) const {
			if(index >= subfields.size() || subfields[index] == NULL) {
				throw logic_error("Field.getSubfield(): invalid index");
			}
			return *subfields[index];
		}


		/**
		 * Returns the Subfield in the Field at index if it exists.  It is an error
		 * if the index is invalid or it contains a NULL Subfield.  The returned Subfield
		 * is not const.
		 *
		 * index: index of the Subfield to be returned.
		 */
		Subfield& Field::getSubfield(size_t index) {
			if(index >= subfields.size() || subfields[index] == NULL) {
				throw logic_error("Field.getSubfield(): invalid index");
			}
			return *subfields[index];
		}


		/**
		 * Returns the number of Subfields in the Field.
		 */
		size_t Field::subfieldsCount() const {
			return subfields.size();
		}


		/**
		 * Adds a new Subfield to the Field.  The new Subfield will be the last Subfield
		 * in the Field.
		 *
		 * subfield: Subfield to add to the Field.
		 */
		void Field::addSubfield(auto_ptr<Subfield> subfield) {
			subfields.push_back(subfield.release());
		}


		/**
		 * Creates a new Subfield with a single Item and adds it to the Field.  The
		 * new Subfield will be the last Subfield in the Field.
		 *
		 * item: Item to add to the new Subfield.
		 */
		void Field::addSubfield(auto_ptr<Item> item) {
			subfields.push_back(new Subfield(item));
		}


		/**
		 * Creates a new Subfield with a single Item from the value string and adds
		 * it to the Field.  The new Subfield will be the last Subfield in the Field.
		 *
		 * value: a string containing the value of the new Subfield.
		 */
		void Field::addSubfield(string const& value) {
			subfields.push_back(new Subfield(value));
		}


		/**
		 * Adds a new Subfield to the Field.  The new Subfield will be placed at the specified
		 * index unless it is already occupied by another Subfield.  If the index is already
		 * occupied by another Subfield it is an error.  If index is past the end of the
		 * subfield vector, NULL elements will be added between the end of the vector
		 * and index.
		 *
		 * subfield: Subfield to add to the Field.
		 * subfieldIndex: position of the new Subfield in the Field.
		 */
		void Field::insertSubfield(auto_ptr<Subfield> subfield, size_t subfieldIndex) {
			while(subfields.size() <= subfieldIndex) {
				subfields.push_back(NULL);
			}
			if(subfields[subfieldIndex] != NULL) {
				throw logic_error("Field.insertSubfield(): cannot replace existing Subfield");
			}
			subfields[subfieldIndex] = subfield.release();
		}


		/**
		 * Adds a new Item to the Field.  If there is no Subfield at subfieldIndex,
		 * a new Subfield is added at that index.  The new Item is added at itemIndex
		 * unless it is already occupied by another Item.  If the index is already
		 * occupied by another Item it is an error.  If subfieldIndex is past the end
		 * of the subfield vector, NULL elements will be added between the end of the
		 * vector and subfieldIndex.  If itemIndex is past the end of the item vector,
		 * NULL elements will be added between the end of the vector and itemIndex.
		 *
		 * item: Item to add.
		 * subfieldIndex: index of the Subfield in the Field.
		 * itemIndex: index of the Item in the Subfield.
		 */
		void Field::insertItem(auto_ptr<Item> item, size_t subfieldIndex, size_t itemIndex) {
			if(subfieldIndex >= subfields.size() || subfields[subfieldIndex] == NULL) {
				insertSubfield(auto_ptr<Subfield>(new Subfield()), subfieldIndex);
			}
			Subfield& subfield = *subfields[subfieldIndex];
			subfield.insertItem(item, itemIndex);
		}


		void Field::appendItem(string const& value, size_t subfieldIndex, size_t itemIndex) {
			if(subfieldIndex >= subfields.size() || subfields[subfieldIndex] == NULL) {
				insertSubfield(auto_ptr<Subfield>(new Subfield()), subfieldIndex);
			}
			Subfield& subfield = *subfields[subfieldIndex];
			subfield.appendItem(value, itemIndex);
		}


		/**
		 * Equality comparison operator.  Returns true if the fieldIds are the same.
		 *
		 * field: Field to compare.
		 */
		bool Field::operator==(Field const& field) const {
			return this->fieldId == field.getFieldID();
		}


		/**
		 * Inequality comparison operator.  Returns the opposite of the equality comparison
		 * operator.
		 *
		 * field: Field to compare.
		 */
		bool Field::operator!=(Field const& field) const {
			return !(*this == field);
		}


		/**
		 * Destructor.
		 */
		Field::~Field() {
			deleteContents<Subfield*>(subfields);
		}


		/**
		 * Returns true if the vector of subfields contains a NULL pointer.
		 */
		bool Field::hasMissingSubfields() const {
			for(vector<Subfield*>::const_iterator it = subfields.begin(); it != subfields.end(); it++) {
				Subfield* subfield = *it;
				if(subfield == NULL) {
					return true;
				}
			}
			return false;
		}


		/**
		 * Parses Part 1 binary data representing a binary image field.
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Field::decodeBinaryImageField(string const& bytes) {
			subfields.push_back(new Subfield(bytes, BinaryImageSubfield));
		}


		/**
		 * Parses Part 1 binary data representing a binary U8 field.  bytes must contain
		 * at least 1 character.
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Field::decodeBinaryU8Field(string const& bytes) {
			if(bytes.empty()) {
				throw logic_error("Field.decodeBinaryU8Field(): invalid argument");
				//binary U8 field must contain at least 1 subfield
				//and each binary U8 subfield must be 1 byte
			}
			for(int i = 0; i < bytes.size(); i++) {
				subfields.push_back(new Subfield(string(bytes, i, 1), BinaryU8Subfield));
			}
		}


		/**
		 * Parses Part 1 binary data representing a binary U16 field.  bytes must contain
		 * at least 2 characters and its length must be divisible by 2.
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Field::decodeBinaryU16Field(string const& bytes) {
			if(bytes.empty() || bytes.size() % 2 != 0) {
				throw logic_error("Field.decodeBinaryU16Field(): invalid argument");
				//binary U16 field must contain at least 1 subfield
				//and each binary U16 subfield must be 2 bytes
			}
			for(int i = 0; i < bytes.size(); i += 2) {
				subfields.push_back(new Subfield(string(bytes, i, 2), BinaryU16Subfield));
			}
		}


		/**
		 * Parses Part 1 binary data representing a binary U32 field.  bytes must contain
		 * at least 4 characters and its length must be divisible by 4.
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Field::decodeBinaryU32Field(string const& bytes) {
			if(bytes.empty() || bytes.size() % 4 != 0) {
				throw logic_error("Field.decodeBinaryU32Field(): invalid argument");
				//binary U32 field must contain at least 1 subfield
				//and each binary U32 subfield must be 4 bytes
			}
			for(int i = 0; i < bytes.size(); i += 4) {
				subfields.push_back(new Subfield(string(bytes, i, 4), BinaryU32Subfield));
			}
		}


		/**
		 * Parses Part 1 binary data representing a binary vector field.  bytes must
		 * contain at least 5 characters and its length must be divisible by 5.
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Field::decodeBinaryVectorField(string const& bytes) {
			if(bytes.empty() || bytes.size() % 5 != 0) {
				throw logic_error("Field.decodeBinaryVectorField(): invalid argument");
				//binary vector field must contain at least 1 subfield
				//and each binary vector subfield must be 5 bytes
			}
			for(int i = 0; i < bytes.size(); i += 5) {
				subfields.push_back(new Subfield(string(bytes, i, 5), BinaryVectorSubfield));
			}
		}


		/**
		 * Parses Part 1 binary data representing a tagged ASCII field.  String should
		 * not be terminated with the field separator character (GS).
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Field::decodeTaggedAsciiField(string const& bytes) {
			auto_ptr<vector<string> > subfieldBytes = split(bytes, RS);
			for(vector<string>::const_iterator it = subfieldBytes->begin(); it != subfieldBytes->end(); it++) {
				string const& s = *it;
				subfields.push_back(new Subfield(s, TaggedAsciiSubfield));
			}
		}


		/**
		 * Parses Part 1 binary data representing a tagged image field.  String should
		 * not be terminated with the field separator character (GS).
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Field::decodeTaggedImageField(string const& bytes) {
			subfields.push_back(new Subfield(bytes, TaggedImageSubfield));
		}


		/**
		 * Creates a string containing Part 1 data representing a binary image field.
		 * Field must contain exactly 1 Subfield.
		 */
		auto_ptr<string> Field::encodeBinaryImageField() const {
			if(subfields.size() != 1) {
				throw logic_error("Field.encodeBinaryImageField(): wrong number of Subfields");
				//there must be exactly 1 subfield in an image field
			}
			return subfields[0]->toBytesForFile(BinaryImageSubfield);
		}


		/**
		 * Creates a string containing Part 1 data representing a binary U8 field.
		 * Field must contain at least 1 Subfield.
		 */
		auto_ptr<string> Field::encodeBinaryU8Field() const {
			if(subfields.empty()) {
				throw logic_error("Field.encodeBinaryU8Field(): wrong number of Subfields");
				//there must be at least 1 subfield in an binary U8 field
			}
			auto_ptr<string> bytes(new string);
			for(vector<Subfield*>::const_iterator it = subfields.begin(); it != subfields.end(); it++) {
				Subfield const& subfield = *(*it);
				*bytes += *(subfield.toBytesForFile(BinaryU8Subfield));
			}
			return bytes;
		}


		/**
		 * Creates a string containing Part 1 data representing a binary U16 field.
		 * Field must contain at least 1 Subfield.
		 */
		auto_ptr<string> Field::encodeBinaryU16Field() const {
			if(subfields.empty()) {
				throw logic_error("Field.encodeBinaryU16Field(): wrong number of Subfields");
				//there must be at least 1 subfield in an binary U16 field
			}
			auto_ptr<string> bytes(new string);
			for(vector<Subfield*>::const_iterator it = subfields.begin(); it != subfields.end(); it++) {
				Subfield const& subfield = *(*it);
				*bytes += *(subfield.toBytesForFile(BinaryU16Subfield));
			}
			return bytes;
		}


		/**
		 * Creates a string containing Part 1 data representing a binary U16 field.
		 * Field must contain at least 1 Subfield.
		 */
		auto_ptr<string> Field::encodeBinaryU32Field() const {
			if(subfields.empty()) {
				throw logic_error("Field.encodeBinaryU32Field(): wrong number of Subfields");
				//there must be at least 1 subfield in an binary U32 field
			}
			auto_ptr<string> bytes(new string);
			for(vector<Subfield*>::const_iterator it = subfields.begin(); it != subfields.end(); it++) {
				Subfield const& subfield = *(*it);
				*bytes += *(subfield.toBytesForFile(BinaryU32Subfield));
			}
			return bytes;
		}


		/**
		 * Creates a string containing Part 1 data representing a binary vector field.
		 * Field must contain at least 1 Subfield.
		 */
		auto_ptr<string> Field::encodeBinaryVectorField() const {
			if(subfields.empty()) {
				throw logic_error("Field.encodeBinaryVectorField(): wrong number of Subfields");
				//there must be at least 1 subfield in an binary vector field
			}
			auto_ptr<string> bytes(new string);
			for(vector<Subfield*>::const_iterator it = subfields.begin(); it != subfields.end(); it++) {
				Subfield const& subfield = *(*it);
				*bytes += *(subfield.toBytesForFile(BinaryVectorSubfield));
			}
			return bytes;
		}


		/**
		 * Creates a string containing Part 1 data representing a tagged ASCII field.
		 * Created string is terminated with the field separator character (GS).  Field
		 * must contain at least 1 Subfield.
		 */
		auto_ptr<string> Field::encodeTaggedAsciiField() const {
			if(subfields.empty()) {
				throw logic_error("Field.encodeTaggedAsciiField(): wrong number of Subfields");
				//there must be at least 1 subfield in a tagged ascii field
			}
			auto_ptr<string> bytes(new string);
			*bytes += fieldId.toBytesForFile();
			for(vector<Subfield*>::const_iterator it = subfields.begin(); it != subfields.end(); it++) {
				Subfield const& subfield = *(*it);
				*bytes += *(subfield.toBytesForFile(TaggedAsciiSubfield));
			}
			bytes->resize(bytes->size() - 1);
			bytes->push_back(GS);
			return bytes;
		}


		/**
		 * Creates a string containing Part 1 data representing a tagged image field.
		 * Created string is terminated with the field separator character (GS).  Field
		 * must contain exactly 1 Subfield.
		 */
		auto_ptr<string> Field::encodeTaggedImageField() const {
			if(subfields.size() != 1) {
				throw logic_error("Field.encodeTaggedImageField(): wrong number of Subfields");
				//there must be exactly 1 subfield in an image field
			}
			auto_ptr<string> bytes(new string);
			*bytes += fieldId.toBytesForFile();
			*bytes += *(subfields[0]->toBytesForFile(TaggedImageSubfield));
			bytes->resize(bytes->size() - 1);
			bytes->push_back(GS);
			return bytes;
		}
	}
}
