#ifndef SUBFIELD_HXX
#define SUBFIELD_HXX

#include "part1/Item.hxx"
#include "utils.hxx"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace convert {
	namespace part1 {
		using namespace std;

		enum SubfieldType {
			BinaryImageSubfield,
			BinaryU8Subfield,
			BinaryU16Subfield,
			BinaryU32Subfield,
			BinaryVectorSubfield,
			TaggedAsciiSubfield,
			TaggedImageSubfield
		};

		class Subfield {
		public:
			Subfield();
			explicit Subfield(auto_ptr<Item> item);
			explicit Subfield(string const& itemValue);
			Subfield(string const& bytes, SubfieldType subfieldType);
			Subfield(Subfield const& subfield);
			size_t getLength(SubfieldType subfieldType) const;
			auto_ptr<string> toBytesForFile(SubfieldType subfieldType) const;
			Item const& getItem(size_t index) const;
			size_t itemsCount() const;
			void addItem(auto_ptr<Item> item);
			void addItem(string const& value);
			void insertItem(auto_ptr<Item> item, size_t index);
			void appendItem(string const& value, size_t index);
			~Subfield();

		private:
			bool hasMissingItems() const;
			void decodeBinaryImageSubfield(string const& bytes);
			void decodeBinaryU8Subfield(string const& bytes);
			void decodeBinaryU16Subfield(string const& bytes);
			void decodeBinaryU32Subfield(string const& bytes);
			void decodeBinaryVectorSubfield(string const& bytes);
			void decodeTaggedAsciiSubfield(string const& bytes);
			void decodeTaggedImageSubfield(string const& bytes);
			auto_ptr<string> encodeBinaryImageSubfield() const;
			auto_ptr<string> encodeBinaryU8Subfield() const;
			auto_ptr<string> encodeBinaryU16Subfield() const;
			auto_ptr<string> encodeBinaryU32Subfield() const;
			auto_ptr<string> encodeBinaryVectorSubfield() const;
			auto_ptr<string> encodeTaggedAsciiSubfield() const;
			auto_ptr<string> encodeTaggedImageSubfield() const;

			vector<Item*> items;
		};
	}
}

#endif
