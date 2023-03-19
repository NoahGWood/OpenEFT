#include "part1/TaggedRecord.hxx"
#include "Errors.hxx"
#include "utils.hxx"

#include <boost/regex.hpp>

#include <sstream>
#include <stdexcept>

namespace convert {
	namespace part1 {
		using namespace std;

		TaggedRecord::TaggedRecord(RecordType recordType, string& bytes, ValidationLevel vl, bool ebts)
		: Record(recordType) {
			size_t recordLength = findLengthField(bytes);
			if(recordLength > bytes.size()) {
				throw ParseError("Tagged record has an incorrect length field value");
				//recordLength is greater than size of bytes
			}
			findFields(string(bytes.begin(), bytes.begin() + recordLength));
			bytes.erase(bytes.begin(), bytes.begin() + recordLength);
			validate(vl, ebts);
		}

		TaggedRecord::TaggedRecord(part2::Record const& part2Record)
		: Record(part2Record) {}

		auto_ptr<string> TaggedRecord::toBytesForFile() {
			calculateLength();

			auto_ptr<string> bytes(new string);
			for(list<Field*>::const_iterator it = fields.begin(); it != fields.end(); it++) {
				if((*it)->getFieldID().getFieldNumber() == 999) {
					*bytes += *((*it)->toBytesForFile(TaggedImageField));
				} else {
					*bytes += *((*it)->toBytesForFile(TaggedAsciiField));
				}
			}
			bytes->resize(bytes->size() - 1);
			bytes->push_back(FS);
			return bytes;
		}

		size_t TaggedRecord::findLengthField(string const& bytes) const {
			stringstream ss;
			ss << "\\A" << recordType << "\\.[0]*1:([0-9]+)" << GS;
			boost::regex re(ss.str());
			string::const_iterator start, end;
			start = bytes.begin();
			end = bytes.end();
			boost::match_results<string::const_iterator> results;
			boost::match_flag_type flags = boost::match_default;
			if(!regex_search(start, end, results, re, flags)) {
				throw ParseError("Tagged record has no LEN field");
				//couldn't find length field
			}
			return stringToSize_t(results.str(1));
		}

		void TaggedRecord::findFields(string const& bytes) {
			size_t startIndex = 0;
			while(true) {
				int endIndex = findNextField(bytes, startIndex);
				if(endIndex == bytes.size()) {
					break;
				} else if(endIndex < bytes.size()) {
					startIndex = endIndex;
				} else {
					throw logic_error("TaggedRecord.findField(): Error parsing fields");
					//something bad happened
				}
			}
		}

		size_t TaggedRecord::findNextField(string const& bytes, size_t startIndex) {
			stringstream imageFieldRegex;
			imageFieldRegex << "\\G" << recordType << "\\.(999):(.*)" << FS << "\\z";
			stringstream lastFieldRegex;
			lastFieldRegex << "\\G" << recordType << "\\.([0-9]{1,3}):([^" << GS << FS << "]*)" << FS;
			stringstream normalFieldRegex;
			normalFieldRegex << "\\G" << recordType << "\\.([0-9]{1,3}):([^" << GS << FS << "]*)" << GS;

			boost::regex re;
			string::const_iterator start, end;
			boost::match_results<string::const_iterator> results;
			boost::match_flag_type flags;

			re = boost::regex(imageFieldRegex.str());
			start = bytes.begin() + startIndex;
			end = bytes.end();
			results = boost::match_results<string::const_iterator>();
			flags = boost::match_default;
			if(regex_search(start, end, results, re, flags)) {
				fields.push_back(new Field(FieldID(recordType, stringToInt(results.str(1))), results.str(2), TaggedImageField));
				return startIndex + results.length(0);
			}

			re = boost::regex(lastFieldRegex.str());
			start = bytes.begin() + startIndex;
			end = bytes.end();
			results = boost::match_results<string::const_iterator>();
			flags = boost::match_default;
			if(regex_search(start, end, results, re, flags)) {
				if(startIndex + results.length(0) != bytes.size()) {
					throw ParseError("Tagged record has data after the record separator character");
					//data after last field
				} else {
					fields.push_back(new Field(FieldID(recordType, stringToInt(results.str(1))), results.str(2), TaggedAsciiField));
					return startIndex + results.length(0);
				}
			}

			re = boost::regex(normalFieldRegex.str());
			start = bytes.begin() + startIndex;
			end = bytes.end();
			results = boost::match_results<string::const_iterator>();
			flags = boost::match_default;
			if(regex_search(start, end, results, re, flags)) {
				if(startIndex + results.length(0) == bytes.size()) {
					throw ParseError("Tagged record has no record separator character");
					//no remaining data, no record separator character
				} else {
					fields.push_back(new Field(FieldID(recordType, stringToInt(results.str(1))), results.str(2), TaggedAsciiField));
					return startIndex + results.length(0);
				}
			}

			//none of the regexes matched, invalid data
			throw ParseError("Could not parse tagged record field");
		}

		void TaggedRecord::calculateLength() {
			removeField(FieldID(recordType, 1));

			size_t len = 0;
			for(list<Field*>::const_iterator it = fields.begin(); it != fields.end(); it++) {
				if((*it)->getFieldID().getFieldNumber() == 999) {
					len += (*it)->getLength(TaggedImageField);
				} else {
					len += (*it)->getLength(TaggedAsciiField);
				}
			}

			Field dummyLen(FieldID(recordType, 1));
			dummyLen.addSubfield("");
			int numLenChars = intToString(len).size();
			len += dummyLen.getLength(TaggedAsciiField) + numLenChars;
			if(numLenChars != intToString(len).size()) {
				len += 1;
			}
			auto_ptr<Field> lenField(new Field(FieldID(recordType, 1)));
			lenField->addSubfield(intToString(len));
			addField(lenField);
		}
	}
}
