#include "part1/Item.hxx"

namespace convert {
	namespace part1 {
		using namespace std;

		/**
		 * Constructs a new Item from a string.
		 *
		 * value: string containing the value of the Item.  Will not be parsed.
		 */
		Item::Item(string const& value) {
			this->value = value;
		}


		/**
		 * Constructs a new Item from Part 1 binary data.  Data is parsed according
		 * to the specified ItemType.
		 *
		 * bytes: string containing Part 1 binary data.
		 * itemType: ItemType to be used when parsing the data.
		 */
		Item::Item(string const& bytes, ItemType itemType) {
			switch(itemType) {
			case BinaryImageItem:
				decodeBinaryImageItem(bytes);
				break;
			case BinaryU8Item:
				decodeBinaryU8Item(bytes);
				break;
			case BinaryU16Item:
				decodeBinaryU16Item(bytes);
				break;
			case BinaryU32Item:
				decodeBinaryU32Item(bytes);
				break;
			case TaggedAsciiItem:
				decodeTaggedAsciiItem(bytes);
				break;
			case TaggedImageItem:
				decodeTaggedImageItem(bytes);
				break;
			}
		}


		/**
		 * Returns the length of the Item in bytes.  The returned length is equal to
		 * the size of the string returned by toBytesForFile(itemType).
		 *
		 * itemType: ItemType to be used when determining the length of the Item.
		 */
		size_t Item::getLength(ItemType itemType) const {
			switch(itemType) {
				case BinaryImageItem:
					return value.size();
				case BinaryU8Item:
					return 1;
				case BinaryU16Item:
					return 2;
				case BinaryU32Item:
					return 4;
				case TaggedAsciiItem:
					return value.size() + 1;
				case TaggedImageItem:
					return value.size() + 1;
			}
		}


		void Item::append(string const& value) {
			this->value += value;
		}


		/**
		 * Returns a string containing the Part 1 binary data representing the Item.
		 * The data will be formatted according the specified ItemType.
		 *
		 * itemType: the ItemType to be used to format the Item data.
		 */
		auto_ptr<string> Item::toBytesForFile(ItemType itemType) const {
			switch(itemType) {
				case BinaryImageItem:
					return encodeBinaryImageItem();
				case BinaryU8Item:
					return encodeBinaryU8Item();
				case BinaryU16Item:
					return encodeBinaryU16Item();
				case BinaryU32Item:
					return encodeBinaryU32Item();
				case TaggedAsciiItem:
					return encodeTaggedAsciiItem();
				case TaggedImageItem:
					return encodeTaggedImageItem();
			}
		}


		/**
		 * Returns a string containing a string representation of the Item data.
		 */
		string const& Item::toString() const {
			return value;
		}


		/**
		 * Parses Part 1 binary data representing a binary image item.
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Item::decodeBinaryImageItem(string const& bytes) {
			value = bytes;
		}


		/**
		 * Parses Part 1 binary data representing a binary U8 item.  bytes must contain
		 * exactly 1 character.
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Item::decodeBinaryU8Item(string const& bytes) {
			if(bytes.size() != 1) {
				throw logic_error("Item.decodeBinaryU8Item(): invalid argument");
			}
			unsigned int intValue = bytes[0] & 0xFF;
			value = uintToString(intValue);
		}


		/**
		 * Parses Part 1 binary data representing a binary U16 item.  bytes must contain
		 * exactly 2 characters.
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Item::decodeBinaryU16Item(string const& bytes) {
			if(bytes.size() != 2) {
				throw logic_error("Item.decodeBinaryU16Item(): invalid argument");
			}
			unsigned int intValue = ((bytes[0] & 0xFF) << 8) | (bytes[1] & 0xFF);
			value = uintToString(intValue);
		}


		/**
		 * Parses Part 1 binary data representing a binary U32 item.  bytes must contain
		 * exactly 4 characters.
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Item::decodeBinaryU32Item(string const& bytes) {
			if(bytes.size() != 4) {
				throw logic_error("Item.decodeBinaryU32Item(): invalid argument");
			}
			unsigned int intValue = ((bytes[0] & 0xFF) << 24) | ((bytes[1] & 0xFF) << 16) | ((bytes[2] & 0xFF) << 8) | (bytes[3] & 0xFF);
			value = uintToString(intValue);
		}


		/**
		 * Parses Part 1 binary data representing a tagged ASCII item.  String should
		 * not be terminated with the item separator character (US).
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Item::decodeTaggedAsciiItem(string const& bytes) {
			value = bytes;
		}


		/**
		 * Parses Part 1 binary data representing a tagged image item.  String should
		 * not be terminated with the item separator character (US).
		 *
		 * bytes: string containing Part 1 data.
		 */
		void Item::decodeTaggedImageItem(string const& bytes) {
			value = bytes;
		}


		/**
		 * Creates a string containing Part 1 data representing a binary image item.
		 */
		auto_ptr<string> Item::encodeBinaryImageItem() const {
			return auto_ptr<string>(new string(value));
		}


		/**
		 * Creates a string containing Part 1 data representing a binary U8 item.
		 */
		auto_ptr<string> Item::encodeBinaryU8Item() const {
			unsigned int intValue = stringToUInt(value);
			auto_ptr<string> bytes(new string);
			bytes->push_back((byte) intValue);
			return bytes;
		}


		/**
		 * Creates a string containing Part 1 data representing a binary U16 item.
		 */
		auto_ptr<string> Item::encodeBinaryU16Item() const {
			unsigned int intValue = stringToUInt(value);
			auto_ptr<string> bytes(new string);
			bytes->push_back((byte) (intValue >> 8));
			bytes->push_back((byte) intValue);
			return bytes;
		}


		/**
		 * Creates a string containing Part 1 data representing a binary U16 item.
		 */
		auto_ptr<string> Item::encodeBinaryU32Item() const {
			unsigned int intValue = stringToUInt(value);
			auto_ptr<string> bytes(new string);
			bytes->push_back((byte) (intValue >> 24));
			bytes->push_back((byte) (intValue >> 16));
			bytes->push_back((byte) (intValue >> 8));
			bytes->push_back((byte) intValue);
			return bytes;
		}


		/**
		 * Creates a string containing Part 1 data representing a tagged ASCII item.
		 * Created string is terminated with the item separator character (US).
		 */
		auto_ptr<string> Item::encodeTaggedAsciiItem() const {
			return auto_ptr<string>(new string(value + US));
		}


		/**
		 * Creates a string containing Part 1 data representing a tagged image item.
		 * Created string is terminated with the item separator character (US).
		 */
		auto_ptr<string> Item::encodeTaggedImageItem() const {
			return auto_ptr<string>(new string(value + US));
		}
	}
}
