#ifndef FIELD_HXX
#define FIELD_HXX

#include "part1/FieldID.hxx"
#include "part1/Item.hxx"
#include "part1/Subfield.hxx"
#include "utils.hxx"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace convert {
	namespace part1 {
		using namespace std;

		enum FieldType {
			BinaryImageField,
			BinaryU8Field,
			BinaryU16Field,
			BinaryU32Field,
			BinaryVectorField,
			TaggedAsciiField,
			TaggedImageField
		};

		class Field {
		public:
			explicit Field(FieldID const& fieldId);
			Field(FieldID const& fieldId, auto_ptr<Subfield> subfield);
			Field(FieldID const& fieldId, auto_ptr<Item> item);
			Field(FieldID const& fieldId, string const& itemValue);
			Field(FieldID const& fieldId, string const& bytes, FieldType fieldType);
			Field(Field const& field);
			size_t getLength(FieldType fieldType) const;
			FieldID const& getFieldID() const;
			auto_ptr<string> toBytesForFile(FieldType fieldType) const;
			Subfield const& getSubfield(size_t index) const;
			Subfield& getSubfield(size_t index);
			size_t subfieldsCount() const;
			void addSubfield(auto_ptr<Subfield> subfield);
			void addSubfield(auto_ptr<Item> item);
			void addSubfield(string const& value);
			void insertSubfield(auto_ptr<Subfield> subfield, size_t subfieldIndex);
			void insertItem(auto_ptr<Item> item, size_t subfieldIndex, size_t itemIndex);
			void appendItem(string const& value, size_t subfieldIndex, size_t itemIndex);
			bool operator==(Field const& field) const;
			bool operator!=(Field const& field) const;
			~Field();

		private:
			bool hasMissingSubfields() const;
			void decodeBinaryImageField(string const& bytes);
			void decodeBinaryU8Field(string const& bytes);
			void decodeBinaryU16Field(string const& bytes);
			void decodeBinaryU32Field(string const& bytes);
			void decodeBinaryVectorField(string const& bytes);
			void decodeTaggedAsciiField(string const& bytes);
			void decodeTaggedImageField(string const& bytes);
			auto_ptr<string> encodeBinaryImageField() const;
			auto_ptr<string> encodeBinaryU8Field() const;
			auto_ptr<string> encodeBinaryU16Field() const;
			auto_ptr<string> encodeBinaryU32Field() const;
			auto_ptr<string> encodeBinaryVectorField() const;
			auto_ptr<string> encodeTaggedAsciiField() const;
			auto_ptr<string> encodeTaggedImageField() const;

			FieldID fieldId;
			vector<Subfield*> subfields;
		};
	}
}

#endif
