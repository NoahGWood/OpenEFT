#include "part1/Subfield.hxx"

namespace convert {
	namespace part1 {
		using namespace std;

		/**
		 * Constructs a new empty Subfield.
		 */
		Subfield::Subfield() {}


		/**
		 * Constructs a new Subfield with a single Item.
		 *
		 * item: Item to add to the Subfield.
		 */
		Subfield::Subfield(auto_ptr<Item> item) {
			addItem(item);
		}


		/**
		 * Constructs a new Subfield with a single Item containing itemValue.
		 *
		 * itemValue: string containing the value of the Item to be created.
		 */
		Subfield::Subfield(string const& itemValue) {
			addItem(itemValue);
		}


		/**
		 * Constructs a new Subfield from Part 1 binary data.  Data is parsed according
		 * to the specified SubfieldType.
		 *
		 * bytes: string containing Part 1 binary data.
		 * subfieldType: SubfieldType to be used when parsing the data.
		 */
		Subfield::Subfield(string const& bytes, SubfieldType subfieldType) {
			switch(subfieldType) {
			case BinaryImageSubfield:
				decodeBinaryImageSubfield(bytes);
				break;
			case BinaryU8Subfield:
				decodeBinaryU8Subfield(bytes);
				break;
			case BinaryU16Subfield:
				decodeBinaryU16Subfield(bytes);
				break;
			case BinaryU32Subfield:
				decodeBinaryU32Subfield(bytes);
				break;
			case BinaryVectorSubfield:
				decodeBinaryVectorSubfield(bytes);
				break;
			case TaggedAsciiSubfield:
				decodeTaggedAsciiSubfield(bytes);
				break;
			case TaggedImageSubfield:
				decodeTaggedImageSubfield(bytes);
				break;
			}
		}


		/**
		 * Copy constructor.
		 */
		Subfield::Subfield(Subfield const& subfield) {
			//Don't copy Subfields with NULL Items
			for(int i = 0; i < subfield.itemsCount(); i++) {
				addItem(auto_ptr<Item>(new Item(subfield.getItem(i))));
			}
		}


		/**
		 * Returns the length of the Subfield in bytes.  The returned length is equal to
		 * the size of the string returned by toBytesForFile(subfieldType).  It is
		 * an error if the Subfield contains NULL Items.
		 *
		 * subfieldType: SubfieldType to be used when determining the length of the
		 * Subfield.
		 */
		size_t Subfield::getLength(SubfieldType subfieldType) const {
			if(hasMissingItems()) {
				throw logic_error("Subfield.getLength(): Subfield has missing Items");
			}
			ItemType itemType;
			switch(subfieldType) {
			case BinaryImageSubfield:
				itemType = BinaryImageItem;
				break;
			case BinaryU8Subfield:
				itemType = BinaryU8Item;
				break;
			case BinaryU16Subfield:
				itemType = BinaryU16Item;
				break;
			case BinaryU32Subfield:
				itemType = BinaryU32Item;
				break;
			case BinaryVectorSubfield:
				if(items.size() != 3) {
					throw logic_error("Subfield.getLength(): wrong number of Items for BinaryVectorSubfield");
				}
				return items[0]->getLength(BinaryU16Item) + items[1]->getLength(BinaryU16Item) + items[2]->getLength(BinaryU8Item);
			case TaggedAsciiSubfield:
				itemType = TaggedAsciiItem;
				break;
			case TaggedImageSubfield:
				itemType = TaggedImageItem;
				break;
			}
			size_t length = 0;
			for(vector<Item*>::const_iterator it = items.begin(); it != items.end(); it++) {
				Item const& item = *(*it);
				length += item.getLength(itemType);
			}
			return length;
		}


		/**
		 * Returns a string containing the Part 1 binary data representing the Subfield.
		 * The data will be formatted according the specified SubfieldType.  It is
		 * an error if the Subfield contains NULL Items.
		 *
		 * subfieldType: the SubfieldType to be used to format the Subfield data.
		 */
		auto_ptr<string> Subfield::toBytesForFile(SubfieldType subfieldType) const {
			if(hasMissingItems()) {
				throw logic_error("Subfield.toBytesForFile(): Subfield has missing Items");
			}
			switch(subfieldType) {
			case BinaryImageSubfield:
				return encodeBinaryImageSubfield();
			case BinaryU8Subfield:
				return encodeBinaryU8Subfield();
			case BinaryU16Subfield:
				return encodeBinaryU16Subfield();
			case BinaryU32Subfield:
				return encodeBinaryU32Subfield();
			case BinaryVectorSubfield:
				return encodeBinaryVectorSubfield();
			case TaggedAsciiSubfield:
				return encodeTaggedAsciiSubfield();
			case TaggedImageSubfield:
				return encodeTaggedImageSubfield();
			}
		}


		/**
		 * Returns the Item in the Subfield at index if it exists.  It is an error
		 * if the index is invalid or it contains a NULL Item.
		 *
		 * index: index of the Item to be returned.
		 */
		Item const& Subfield::getItem(size_t index) const {
			if(index >= items.size() || items[index] == NULL) {
				throw logic_error("Subfield.getItem(): invalid index");
			}
			return *items[index];
		}


		/**
		 * Returns the number of Items in the Subfield.
		 */
		size_t Subfield::itemsCount() const {
			return items.size();
		}


		/**
		 * Adds a new Item to the Subfield.  The new Item will be the last Item in
		 * the Subfield.
		 *
		 * item: Item to add to the Subfield.
		 */
		void Subfield::addItem(auto_ptr<Item> item) {
			items.push_back(item.release());
		}


		/**
		 * Creates a new Item from the value string and adds it to the Subfield.  The
		 * new Item will be the last Item in the Subfield.
		 *
		 * value: a string containing the value of the new Item.
		 */
		void Subfield::addItem(string const& value) {
			items.push_back(new Item(value));
		}


		/**
		 * Adds a new Item to the Subfield.  The new Item will be placed at the specified
		 * index unless it is already occupied by another Item.  If the index is already
		 * occupied by another Item it is an error.  If index is past the end of the
		 * item vector, NULL elements will be added between the end of the vector and
		 * index.
		 *
		 * item: Item to add to the Subfield.
		 * index: position of the new Item in the Subfield.
		 */
		void Subfield::insertItem(auto_ptr<Item> item, size_t index) {
			while(items.size() <= index) {
				items.push_back(NULL);
			}
			if(items[index] != NULL) {
				throw logic_error("Subfield.insertItem(): cannot replace existing Item");
			}
			items[index] = item.release();
		}


		void Subfield::appendItem(string const& value, size_t index) {
			if(index >= items.size() || items[index] == NULL) {
				insertItem(auto_ptr<Item>(new Item("")), index);
			}
			Item& item = *items[index];
			item.append(value);
		}


		/**
		 * Destructor.
		 */
		Subfield::~Subfield() {
			deleteContents<Item*>(items);
		}


		/**
		 * Returns true if the vector of items contains a NULL pointer.
		 */
		bool Subfield::hasMissingItems() const {
			for(vector<Item*>::const_iterator it = items.begin(); it != items.end(); it++) {
				Item* item = *it;
				if(item == NULL) {
					return true;
				}
			}
			return false;
		}


		/**
		 * Parses Part 1 binary data representing a binary image subfield.
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Subfield::decodeBinaryImageSubfield(string const& bytes) {
			items.push_back(new Item(bytes, BinaryImageItem));
		}


		/**
		 * Parses Part 1 binary data representing a binary U8 subfield.  bytes must
		 * contain exactly 1 character.
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Subfield::decodeBinaryU8Subfield(string const& bytes) {
			if(bytes.size() != 1) {
				throw logic_error("Subfield.decodeBinaryU8Subfield(): invalid argument");
			}
			items.push_back(new Item(bytes, BinaryU8Item));
		}


		/**
		 * Parses Part 1 binary data representing a binary U16 subfield.  bytes must
		 * contain exactly 2 characters.
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Subfield::decodeBinaryU16Subfield(string const& bytes) {
			if(bytes.size() != 2) {
				throw logic_error("Subfield.decodeBinaryU16Subfield(): invalid argument");
			}
			items.push_back(new Item(bytes, BinaryU16Item));
		}


		/**
		 * Parses Part 1 binary data representing a binary U32 subfield.  bytes must
		 * contain exactly 4 characters.
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Subfield::decodeBinaryU32Subfield(string const& bytes) {
			if(bytes.size() != 4) {
				throw logic_error("Subfield.decodeBinaryU32Subfield(): invalid argument");
			}
			items.push_back(new Item(bytes, BinaryU32Item));
		}


		/**
		 * Parses Part 1 binary data representing a binary vector subfield.  bytes
		 * must contain exactly 5 characters.
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Subfield::decodeBinaryVectorSubfield(string const& bytes) {
			if(bytes.size() != 5) {
				throw logic_error("Subfield.decodeBinaryVectorSubfield(): invalid argument");
			}
			items.push_back(new Item(bytes.substr(0, 2), BinaryU16Item));
			items.push_back(new Item(bytes.substr(2, 2), BinaryU16Item));
			items.push_back(new Item(bytes.substr(4, 1), BinaryU8Item));
		}


		/**
		 * Parses Part 1 binary data representing a tagged ASCII subfield.  String
		 * should not be terminated with the subfield separator character (RS).
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Subfield::decodeTaggedAsciiSubfield(string const& bytes) {
			auto_ptr<vector<string> > itemBytes = split(bytes, US);
			for(vector<string>::const_iterator it = itemBytes->begin(); it != itemBytes->end(); it++) {
				string const& s = *it;
				items.push_back(new Item(s, TaggedAsciiItem));
			}
		}


		/**
		 * Parses Part 1 binary data representing a tagged image subfield.  String
		 * should not be terminated with the subfield separator character (RS).
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Subfield::decodeTaggedImageSubfield(string const& bytes) {
			items.push_back(new Item(bytes, TaggedImageItem));
		}


		/**
		 * Creates a string containing Part 1 data representing a binary image subfield.
		 * Subfield must contain exactly 1 Item.
		 */
		auto_ptr<string> Subfield::encodeBinaryImageSubfield() const {
			if(items.size() != 1) {
				throw logic_error("Subfield.encodeBinaryImageSubfield(): wrong number of Items");
			}
			return items[0]->toBytesForFile(BinaryImageItem);
		}


		/**
		 * Creates a string containing Part 1 data representing a binary U8 subfield.
		 * Subfield must contain exactly 1 Item.
		 */
		auto_ptr<string> Subfield::encodeBinaryU8Subfield() const {
			if(items.size() != 1) {
				throw logic_error("Subfield.encodeBinaryU8Subfield(): wrong number of Items");
			}
			return items[0]->toBytesForFile(BinaryU8Item);
		}


		/**
		 * Creates a string containing Part 1 data representing a binary U16 subfield.
		 * Subfield must contain exactly 1 Item.
		 */
		auto_ptr<string> Subfield::encodeBinaryU16Subfield() const {
			if(items.size() != 1) {
				throw logic_error("Subfield.encodeBinaryU16Subfield(): wrong number of Items");
			}
			return items[0]->toBytesForFile(BinaryU16Item);
		}


		/**
		 * Creates a string containing Part 1 data representing a binary U32 subfield.
		 * Subfield must contain exactly 1 Item.
		 */
		auto_ptr<string> Subfield::encodeBinaryU32Subfield() const {
			if(items.size() != 1) {
				throw logic_error("Subfield.encodeBinaryU32Subfield(): wrong number of Items");
			}
			return items[0]->toBytesForFile(BinaryU32Item);
		}


		/**
		 * Creates a string containing Part 1 data representing a binary vector subfield.
		 * Subfield must contain exactly 3 Items.
		 */
		auto_ptr<string> Subfield::encodeBinaryVectorSubfield() const {
			if(items.size() != 3) {
				throw logic_error("Subfield.encodeBinaryVectorSubfield(): wrong number of Items");
			}
			auto_ptr<string> bytes(new string);
			*bytes += *(items[0]->toBytesForFile(BinaryU16Item));
			*bytes += *(items[1]->toBytesForFile(BinaryU16Item));
			*bytes += *(items[2]->toBytesForFile(BinaryU8Item));
			return bytes;
		}


		/**
		 * Creates a string containing Part 1 data representing a tagged ASCII subfield.
		 * Created string is terminated with the subfield separator character (RS).
		 * Subfield must contain at least 1 Item.
		 */
		auto_ptr<string> Subfield::encodeTaggedAsciiSubfield() const {
			if(items.size() < 1) {
				throw logic_error("Subfield.encodeTaggedAsciiSubfield(): wrong number of Items");
			}
			auto_ptr<string> bytes(new string);
			for(vector<Item*>::const_iterator it = items.begin(); it != items.end(); it++) {
				Item const& item = *(*it);
				*bytes += *(item.toBytesForFile(TaggedAsciiItem));
			}
			bytes->resize(bytes->size() - 1);
			bytes->push_back(RS);
			return bytes;
		}


		/**
		 * Creates a string containing Part 1 data representing a tagged image subfield.
		 * Created string is terminated with the subfield separator character (RS).
		 * Subfield must contain exactly 1 Item.
		 */
		auto_ptr<string> Subfield::encodeTaggedImageSubfield() const {
			if(items.size() != 1) {
				throw logic_error("Subfield.encodeTaggedImageSubfield(): wrong number of Items");
			}
			auto_ptr<string> bytes(items[0]->toBytesForFile(TaggedImageItem));
			bytes->resize(bytes->size() - 1);
			bytes->push_back(RS);
			return bytes;
		}
	}
}
