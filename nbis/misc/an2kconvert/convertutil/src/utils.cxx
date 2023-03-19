#include "utils.hxx"

#include <iostream>
#include <sstream>

namespace convert {
	using namespace std;

	auto_ptr<vector<string> > split(string const& s, char delim) {
		auto_ptr<vector<string> > strs(new vector<string>());
		string tempStr;
		for(int i = 0; i < s.size(); i++) {
			if(s[i] == delim) {
				strs->push_back(tempStr);
				tempStr = string();
			} else {
				tempStr += s[i];
			}
		}
		strs->push_back(tempStr);
		return strs;
	}

	string intToString(int i) {
		stringstream ss;
		ss << i;
		return ss.str();
	}

	int stringToInt(string const& s) {
		stringstream ss(s);
		int i;
		ss >> i;
		return i;
	}

	string uintToString(unsigned int i) {
		stringstream ss;
		ss << i;
		return ss.str();
	}

	unsigned int stringToUInt(string const& s) {
		stringstream ss(s);
		unsigned int i;
		ss >> i;
		return i;
	}

	string size_tToString(size_t i) {
		stringstream ss;
		ss << i;
		return ss.str();
	}

	size_t stringToSize_t(string const& s) {
		stringstream ss(s);
		size_t i;
		ss >> i;
		return i;
	}
}
