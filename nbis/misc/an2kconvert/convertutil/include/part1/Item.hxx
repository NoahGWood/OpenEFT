#ifndef ITEM_HXX
#define ITEM_HXX

#include "utils.hxx"

#include <memory>
#include <stdexcept>
#include <string>

namespace convert {
	namespace part1 {
		using namespace std;

		enum ItemType {
			BinaryImageItem,
			BinaryU8Item,
			BinaryU16Item,
			BinaryU32Item,
			TaggedAsciiItem,
			TaggedImageItem
		};

		class Item {
		public:
			explicit Item(string const& value);
			Item(string const& bytes, ItemType itemType);
			size_t getLength(ItemType itemType) const;
			void append(string const& value);
			auto_ptr<string> toBytesForFile(ItemType itemType) const;
			string const& toString() const;

		private:
			void decodeBinaryImageItem(string const& bytes);
			void decodeBinaryU8Item(string const& bytes);
			void decodeBinaryU16Item(string const& bytes);
			void decodeBinaryU32Item(string const& bytes);
			void decodeTaggedAsciiItem(string const& bytes);
			void decodeTaggedImageItem(string const& bytes);
			auto_ptr<string> encodeBinaryImageItem() const;
			auto_ptr<string> encodeBinaryU8Item() const;
			auto_ptr<string> encodeBinaryU16Item() const;
			auto_ptr<string> encodeBinaryU32Item() const;
			auto_ptr<string> encodeTaggedAsciiItem() const;
			auto_ptr<string> encodeTaggedImageItem() const;

			string value;
		};
	}
}

#endif
