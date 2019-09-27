/* Information Security Class HW1: Encrypt.cpp
   Created by 四資工三甲 B10615031 許晉捷 */

#include <iostream>
#include <string>
#include <map>
#include <exception>
#include <algorithm>

// error messages
#define ARGC_ERROR_MSG "Argument counts error."
#define KEY_NOT_INTEGER_ERROR_MSG "The key of Caesar/Rail-fence must be an integer."
#define KEY_OUT_OF_RANGE_ERROR_MSG "The key of Caesar/Rail-fence is out of range of Int."
#define NO_CYPHER_EXISTS_ERROR_MSG "No such cypher method exists."
#define ROW_TRANSPOSITION_CYPHER_KEY_WITH_NON_DIGITS_ERROR_MSG "The key of Row transposition cypher must be made of numbers only."

// using
using std::string;
using std::map;
using std::invalid_argument;
using std::out_of_range;

// function pointer define
typedef string (*CypherFunction)(string, string);

// some easy functions
#define is_lower_case(c) ((c) >= 97 && (c) <= 122)
#define is_upper_case(c) ((c) >= 65 && (c) <= 90)
#define to_lowercase(str) std::transform(str.begin(), str.end(), str.begin(), [] (unsigned char c) { return std::tolower(c); });

// cypher methods
string caesar(string, string) throw (invalid_argument, out_of_range);
string playfair(string, string);
string vernam(string, string);
string row(string, string);
string railFence(string, string);

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

string row(string key, string plainText) {
	int textLength = plainText.length();
	int keyLength = key.length();
	int keyIdx = 0;
	int* row = (int*)malloc(sizeof(int) * keyLength);
	string ret = "";

	// TODO: row transposition cypher
	for (int k = 0; k < textLength; ++k) {
		int rowIdx = k / keyLength;
		int colIdx = k % keyLength;
		row[colIdx] = plainText[k];
	}

	return "row";
}

string railFence(string key, string plainText) {
	return "rail_fence";
}