#ifndef BASE64_HXX
#define BASE64_HXX

#include <boost/regex.hpp>

#include <stdexcept>
#include <string>

namespace convert {
	using namespace std;

	class Base64 {
	public:
		static string encode(string const& binStr);
		static string decode(string const& b64Str);

	private:
		static string padBinStr(string const& binStr);
		static string sanitizeB64Str(string const& b64Str);
		static bool validateB64Str(string const& b64Str);
		static char decodeB64Char(char b64Char);
		static void encodeTriple(string const& binStr, size_t pos, string& b64Str);
		static void decodeQuad(string const& b64Str, size_t pos, string& binStr);
	};
}

#endif
