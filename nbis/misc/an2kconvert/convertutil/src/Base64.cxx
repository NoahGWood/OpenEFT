#include "Base64.hxx"

namespace convert {
	using namespace std;

	string Base64::encode(string const& binStr) {
		size_t binStrLen = binStr.length();
		string paddedBinStr = padBinStr(binStr);

		string b64Str;
		for(int i = 0; i < paddedBinStr.length(); i += 3) {
			encodeTriple(paddedBinStr, i, b64Str);
		}

		if(binStrLen % 3 == 1) {
			b64Str.resize(b64Str.length() - 2);
			b64Str += "==";
		} else if(binStrLen % 3 == 2) {
			b64Str.resize(b64Str.length() - 1);
			b64Str += "=";
		}
		return b64Str;
	}

	string Base64::decode(string const& b64Str) {
		string sanitizedB64Str = sanitizeB64Str(b64Str);
		if(!validateB64Str(sanitizedB64Str)) {
			throw logic_error("Invalid Base 64 string");
		}

		string binStr;
		for(int i = 0; i < sanitizedB64Str.length(); i += 4) {
			decodeQuad(sanitizedB64Str, i, binStr);
		}

		if(sanitizedB64Str[sanitizedB64Str.length() - 2] == '=') {
			binStr.resize(binStr.size() - 2);
		} else if(sanitizedB64Str[sanitizedB64Str.length() - 1] == '=') {
			binStr.resize(binStr.size() - 1);
		}
		return binStr;
	}

	string Base64::padBinStr(string const& binStr) {
		string paddedBinStr = binStr;
		while(paddedBinStr.length() % 3 != 0) {
			paddedBinStr += '\0';
		}
		return paddedBinStr;
	}

	string Base64::sanitizeB64Str(string const& b64Str) {
		string sanitizedB64Str;
		for(int i = 0; i < b64Str.length(); i++) {
			if(b64Str[i] >= 'A' && b64Str[i] <= 'Z') {
				sanitizedB64Str += b64Str[i];
			}
			if(b64Str[i] >= 'a' && b64Str[i] <= 'z') {
				sanitizedB64Str += b64Str[i];
			}
			if(b64Str[i] >= '0' && b64Str[i] <= '9') {
				sanitizedB64Str += b64Str[i];
			}
			if(b64Str[i] == '+' || b64Str[i] == '/' || b64Str[i] == '=') {
				sanitizedB64Str += b64Str[i];
			}
		}
		return sanitizedB64Str;
	}

	bool Base64::validateB64Str(string const& b64Str) {
		if(b64Str.length() < 0 || b64Str.length() % 4 != 0) {
			return false;
		}
		boost::regex re("^[A-Za-z0-9+/\\s]+[=]{0, 2}$");
		boost::match_results<string::const_iterator> results;
		boost::match_flag_type flags = boost::match_default;
		if(regex_search(b64Str.begin(), b64Str.end(), results, re, flags)) {
			return true;
		} else {
			return false;
		}
	}

	char Base64::decodeB64Char(char b64Char) {
		if(b64Char == '=') {
			return 0;
		}
		if(b64Char >= 'A' && b64Char <= 'Z') {
			return b64Char - 'A';
		}
		if(b64Char >= 'a' && b64Char <= 'z') {
			return b64Char - 'a' + 26;
		}
		if(b64Char >= '0' && b64Char <= '9') {
			return b64Char - '0' + 52;
		}
		if(b64Char == '+') {
			return 62;
		}
		if(b64Char == '/') {
			return 63;
		}
	}

	void Base64::encodeTriple(string const& binStr, size_t pos, string& b64Str) {
		string lookupTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		char triple[3];
		char quad[4];
		triple[0] = binStr[pos];
		triple[1] = binStr[pos + 1];
		triple[2] = binStr[pos + 2];
		quad[0] = (triple[0] &  0xFC) >> 2;
		quad[1] = ((triple[0] & 0x03) << 4) | ((triple[1] & 0xF0) >> 4);
		quad[2] = ((triple[1] & 0x0F) << 2) | ((triple[2] & 0xC0) >> 6);
		quad[3] = (triple[2] & 0x3F);
		for(int i = 0; i < 4; i++) {
			b64Str += lookupTable[quad[i]];
		}
	}

	void Base64::decodeQuad(string const& b64Str, size_t pos, string& binStr) {
		char quad[4];
		char triple[3];
		quad[0] = decodeB64Char(b64Str[pos]);
		quad[1] = decodeB64Char(b64Str[pos + 1]);
		quad[2] = decodeB64Char(b64Str[pos + 2]);
		quad[3] = decodeB64Char(b64Str[pos + 3]);
		triple[0] = ((quad[0] & 0x3F) << 2) | ((quad[1] & 0x30) >> 4);
		triple[1] = ((quad[1] & 0x0F) << 4) | ((quad[2] & 0x3C) >> 2);
		triple[2] = ((quad[2] & 0x03) << 6) | (quad[3] & 0x3F);
		for(int i = 0; i < 3; i++) {
			binStr += triple[i];
		}
	}
}
