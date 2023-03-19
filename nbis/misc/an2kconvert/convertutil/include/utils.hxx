#ifndef UTILS_HXX
#define UTILS_HXX

#include <list>
#include <memory>
#include <stack>
#include <string>
#include <vector>

namespace convert {
	using namespace std;

	auto_ptr<vector<string> > split(string const& s, char delim);
	string intToString(int i);
	int stringToInt(string const& s);
	string uintToString(unsigned int i);
	unsigned int stringToUInt(string const& s);
	string size_tToString(size_t i);
	size_t stringToSize_t(string const& s);

	template<typename t> void deleteContents(list<t>& l) {
		while(!l.empty()) {
			delete l.back();
			l.pop_back();
		}
	}

	template<typename t1, typename t2> void deleteContents(stack<t1, t2>& s) {
		while(!s.empty()) {
			delete s.top();
			s.pop();
		}
	}

	template<typename t> void deleteContents(vector<t>& v) {
		while(!v.empty()) {
			delete v.back();
			v.pop_back();
		}
	}

	const size_t INF = -1;

	typedef /*unsigned*/ char byte;

	const byte FS = (byte) 0x1c;
	const byte GS = (byte) 0x1d;
	const byte RS = (byte) 0x1e;
	const byte US = (byte) 0x1f;
}

#endif
