#include "convert/format/Formatter.hxx"
#include "Base64.hxx"
#include "Errors.hxx"
#include "utils.hxx"

#include <boost/regex.hpp>

#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace convert {
	using namespace std;

	string Formatter::formatPart1(string const& part2Value) const {
		return part2Value;
	}

	string Formatter::formatPart2(string const& part1Value) const {
		return part1Value;
	}

	string Base64Formatter::formatPart1(string const& part2Value) const {
		return Base64::decode(part2Value);
	}

	string Base64Formatter::formatPart2(string const& part1Value) const {
		return Base64::encode(part1Value);
	}

	string DateFormatter::formatPart1(string const& part2Value) const {
		boost::regex re("([0-9]{4})-([0-9]{2})-([0-9]{2})");
		boost::match_results<string::const_iterator> results;
		boost::match_flag_type flags = boost::match_default;
		if(regex_search(part2Value.begin(), part2Value.end(), results, re, flags)) {
			return results.str(1) + results.str(2) + results.str(3);
		} else {
			throw logic_error("Error formatting date");
		}
	}

	string DateFormatter::formatPart2(string const& part1Value) const {
		boost::regex re("([0-9]{4})([0-9]{2})([0-9]{2})");
		boost::match_results<string::const_iterator> results;
		boost::match_flag_type flags = boost::match_default;
		if(regex_search(part1Value.begin(), part1Value.end(), results, re, flags)) {
			return results.str(1) + "-" + results.str(2) + "-" + results.str(3);
		} else {
			throw logic_error("Error formatting date");
		}
	}

	string IDCFormatter::formatPart1(string const& part2Value) const {
		stringstream ss;
		ss << setfill('0') << setw(2) << part2Value;
		return ss.str();
	}

	string IDCFormatter::formatPart2(string const& part1Value) const {
		stringstream ss;
		ss << setfill('0') << setw(2) << part1Value;
		return ss.str();
	}

	ItemSplitFormatter::ItemSplitFormatter(char delimiter, size_t index)
	: delimiter(delimiter), index(index) {}

	string ItemSplitFormatter::formatPart1(string const& part2Value) const {
		if(index == 0) {
			return part2Value;
		} else {
			stringstream ss;
			ss << "," << part2Value;
			return ss.str();
		}
	}

	string ItemSplitFormatter::formatPart2(string const& part1Value) const {
		auto_ptr<vector<string> > strs = split(part1Value, delimiter);
		if(index < strs->size()) {
			return strs->at(index);
		} else {
			throw logic_error("Error splitting item, invalid index");
		}
	}

	string RidgeCountIndicatorFormatter::formatPart1(string const& part2Value) const {
		if(part2Value == "true") {
			return "1";
		} else if(part2Value == "false") {
			return "0";
		} else {
			throw logic_error("Invalid ridge count indicator value");
		}
	}

	string RidgeCountIndicatorFormatter::formatPart2(string const& part1Value) const {
		if(part1Value == "1") {
			return "true";
		} else if(part1Value == "0") {
			return "false";
		} else {
			throw logic_error("Invalid ridge count indicator value");
		}
	}

	string StandardIndicatorFormatter::formatPart1(string const& part2Value) const {
		if(part2Value == "true") {
			return "S";
		} else if(part2Value == "false") {
			return "U";
		} else {
			throw logic_error("Invalid standard indicator value");
		}
	}

	string StandardIndicatorFormatter::formatPart2(string const& part1Value) const {
		if(part1Value == "S") {
			return "true";
		} else if(part1Value == "U") {
			return "false";
		} else {
			throw logic_error("Invalid standard indicator value");
		}
	}

	string UTCDateFormatter::formatPart1(string const& part2Value) const {
		boost::regex re("([0-9]{4})-([0-9]{2})-([0-9]{2})T([0-9]{2}):([0-9]{2}):([0-9]{2})Z");
		boost::match_results<string::const_iterator> results;
		boost::match_flag_type flags = boost::match_default;
		if(regex_search(part2Value.begin(), part2Value.end(), results, re, flags)) {
			return results.str(1) + results.str(2) + results.str(3) + results.str(4) + results.str(5) + results.str(6) + "Z";
		} else {
			throw logic_error("Error formatting UTC date");
		}
	}

	string UTCDateFormatter::formatPart2(string const& part1Value) const {
		boost::regex re("([0-9]{4})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})([0-9]{2})Z");
		boost::match_results<string::const_iterator> results;
		boost::match_flag_type flags = boost::match_default;
		if(regex_search(part1Value.begin(), part1Value.end(), results, re, flags)) {
			return results.str(1) + "-" + results.str(2) + "-" + results.str(3) + "T" + results.str(4) + ":" + results.str(5) + ":" + results.str(6) + "Z";
		} else {
			throw logic_error("Error formatting UTC date");
		}
	}

	string Type8LLFormatter::formatPart2(string const& part1Value) const {
		if(part1Value == "0") {
			return "00";
		}
	}
}
