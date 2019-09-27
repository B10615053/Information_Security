/* Information Security Class HW1: Encrypt.cpp
   Created by 四資工三甲 B10615031 許晉捷 */

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <exception>
#include <algorithm>

// error messages
#define ARGC_ERROR_MSG "Argument counts error."
#define KEY_NOT_INTEGER_ERROR_MSG "The key of Caesar/Rail-fence must be an integer."
#define KEY_OUT_OF_RANGE_ERROR_MSG "The key of Caesar/Rail-fence is out of range of Int."
#define NO_CYPHER_EXISTS_ERROR_MSG "No such cypher method exists."
#define ROW_TRANSPOSITION_CYPHER_KEY_WITH_NONDIGITS_ERROR_MSG "The key of Row transposition cypher must be made of digits (0-9) only."

// using
using std::string;
using std::map;
using std::vector;
using std::invalid_argument;
using std::out_of_range;

// function pointer define
typedef string (*CypherFunction)(string, string);

// some easy functions
#define is_lower_case(c) ((c) >= 97 && (c) <= 122)
#define is_upper_case(c) ((c) >= 65 && (c) <= 90)
#define is_digit(c) ((c) >= 48 && (c) <= 57)
#define to_lowercase(str) std::transform(str.begin(), str.end(), str.begin(), [] (unsigned char c) { return std::tolower(c); });

// user-defined exceptions
class row_transposition_cypher_key_with_nondigits: public std::exception { const char* what() const { return ROW_TRANSPOSITION_CYPHER_KEY_WITH_NONDIGITS_ERROR_MSG; } };

// cypher methods
string caesar(string, string) throw (invalid_argument, out_of_range);
string playfair(string, string);
string vernam(string, string);
string row(string, string) throw (row_transposition_cypher_key_with_nondigits);
string railFence(string, string) throw (invalid_argument, out_of_range);

// map of cypher methods
map<string, CypherFunction> cypherMethods = {
	{"caesar", &caesar},
	{"playfair", &playfair},
	{"vernam", &vernam},
	{"row", &row},
	{"rail_fence", &railFence}
};

int main(int argc, char** argv) {
	// argc must be 4
	if (argc != 4)
		puts(ARGC_ERROR_MSG), exit(EXIT_FAILURE);

	// do the encryption
	try {
		string cypher = argv[1], key = argv[2], plainText = argv[3];

		// to lowercase but still not find out the method -> not exists
		to_lowercase(cypher);
		if (!cypherMethods.count(cypher))
			puts(NO_CYPHER_EXISTS_ERROR_MSG), puts("Only caesar, playfair, vernam, row, and rail_fence allowed."), exit(EXIT_FAILURE);

		string cypherText = cypherMethods[cypher](key, plainText);
		printf(cypherText.c_str());
	}

	// the key of Caesar or Rail-fence is not an integer
	catch (const invalid_argument& e) { puts(KEY_NOT_INTEGER_ERROR_MSG), exit(EXIT_FAILURE); }

	// the key of Caesar or Rail-fence is out of range of Int
	catch (const out_of_range& e) { puts(KEY_OUT_OF_RANGE_ERROR_MSG), exit(EXIT_FAILURE); }

	// the key of Caesar or Rail-fence is out of range of Int
	catch (row_transposition_cypher_key_with_nondigits& e) { puts(ROW_TRANSPOSITION_CYPHER_KEY_WITH_NONDIGITS_ERROR_MSG), exit(EXIT_FAILURE); }

	return 0;
}

string caesar(string key, string plainText) throw (invalid_argument, out_of_range) {
	int integerKey = std::stoi(key);
	int shiftOffset = ((integerKey % 26) + 26) % 26;
	int textLength = plainText.length();
	string ret = "";

	for (int k = 0; k < textLength; ++k) {
		char plainChar = plainText[k];

		// the plain char is a lower-case
		if (is_lower_case(plainChar))
			ret += ((plainChar - 97 + shiftOffset) % 26) + 65;
		// the plain char is an upper-case
		else
			ret += ((plainChar - 65 + shiftOffset) % 26) + 65;
	}

	return ret;
}

string playfair(string key, string plainText) {
	return "playfair";

}

string vernam(string key, string plainText) {
	int textLength = plainText.length();
	int keyLength = key.length();
	int keyIdx = 0;
	string ret = "";

	for (int k = 0; k < textLength; ++k) {
		char plainChar = plainText[k];
		char keyChar = keyIdx < keyLength ? key[keyIdx] : plainText[keyIdx - keyLength];

		int plainCode = is_lower_case(plainChar) ? plainChar - 97 : (is_upper_case(plainChar) ? plainChar - 65 : plainChar - 48);
		int keyCode = is_lower_case(keyChar) ? keyChar - 97 : (is_upper_case(keyChar) ? keyChar - 65 : keyChar - 48);

		char xorred = plainCode ^ keyCode;
		if (xorred >= 0 && xorred <= 25)
			ret += xorred + 65;
		else
			ret += xorred + 65;

		++keyIdx;
	}

	return ret;
}

string row(string key, string plainText) throw (row_transposition_cypher_key_with_nondigits) {
	int textLength = plainText.length();
	int keyLength = key.length();
	map<int, int> orderingMap;
	string ret = "";

	// make the ordering
	for (int k = 0; k < keyLength; ++k) {
		char keyChar = key[k];

		if (is_digit(keyChar))
			orderingMap.insert(std::make_pair(keyChar - 48, k));
		else
			throw row_transposition_cypher_key_with_nondigits();
	}

	// ciphering
	for (map<int, int>::iterator it = orderingMap.begin(); it != orderingMap.end(); ++it) {
		for (int txtIdx = it->second; txtIdx < textLength; txtIdx += keyLength) {
			char plainChar = plainText[txtIdx];

			if (is_lower_case(plainChar))
				ret += plainText[txtIdx] - 32;
			else
				ret += plainText[txtIdx];
		}
	}

	return ret;
}

string railFence(string key, string plainText) throw (invalid_argument, out_of_range) {
	int integerKey = std::stoi(key);

	// if the key is 1, return the plain-text directly
	if (integerKey <= 1)
		return plainText;

	int divisor = (integerKey * 2) - 2;
	int remainder;
	int textLength = plainText.length();
	vector<string> matrix = vector<string>(integerKey);
	string ret = "";

	// build the zigzag'd matrix
	for (int k = 0; k < textLength; ++k) {
		char plainChar = plainText[k];

		remainder = k % divisor;
		if (remainder < integerKey)
			matrix[remainder] += is_lower_case(plainChar) ? plainChar - 32 : plainChar;
		else
			matrix[divisor - remainder] += is_lower_case(plainChar) ? plainChar - 32 : plainChar;
	}

	// make the cipher-text
	for (int k = 0; k < integerKey; ++k)
		ret += matrix[k];

	return ret;
}