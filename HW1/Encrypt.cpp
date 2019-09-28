/* Information Security Class HW1: Encrypt.cpp
   Create by 四資工三甲 B10615031 許晉捷 */

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <exception>
#include <algorithm>

// error messages
#define ARGC_ERROR_MSG "Argument counts error."
#define KEY_NOT_INTEGER_ERROR_MSG "The key of Caesar/Rail-fence must be an integer."
#define KEY_OUT_OF_RANGE_ERROR_MSG "The key of Caesar/Rail-fence is out of range of Int."
#define NO_CIPHER_EXISTS_ERROR_MSG "No such cipher method exists."
#define ROW_TRANSPOSITION_CIPHER_KEY_WITH_NONDIGITS_ERROR_MSG "The key of Row transposition cipher must be made of digits (0-9) only."

// using
using std::string;
using std::map;
using std::set;
using std::vector;
using std::invalid_argument;
using std::out_of_range;

// function pointer define
typedef string (*CipherFunction)(string, string);

// some easy functions
#define is_lower_case(c) ((c) >= 97 && (c) <= 122)
#define is_upper_case(c) ((c) >= 65 && (c) <= 90)
#define is_digit(c) ((c) >= 48 && (c) <= 57)
#define to_lowercase(str) std::transform(str.begin(), str.end(), str.begin(), [] (unsigned char c) { return std::tolower(c); });
#define to_uppercase(str) std::transform(str.begin(), str.end(), str.begin(), [] (unsigned char c) { return std::toupper(c); });

// user-defined exceptions
class row_transposition_cipher_key_with_nondigits: public std::exception { const char* what() const { return ROW_TRANSPOSITION_CIPHER_KEY_WITH_NONDIGITS_ERROR_MSG; } };

// cipher methods
string caesar(string, string) throw (invalid_argument, out_of_range);
string playfair(string, string);
string vernam(string, string);
string row(string, string) throw (row_transposition_cipher_key_with_nondigits);
string railFence(string, string) throw (invalid_argument, out_of_range);

// map of cipher methods
map<string, CipherFunction> cipherMethods = {
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
		string cipher = argv[1], key = argv[2], plainText = argv[3];

		// to lowercase but still not find out the method -> not exists
		to_lowercase(cipher);
		if (!cipherMethods.count(cipher))
			puts(NO_CIPHER_EXISTS_ERROR_MSG), puts("Only caesar, playfair, vernam, row, and rail_fence allowed."), exit(EXIT_FAILURE);

		string cipherText = cipherMethods[cipher](key, plainText);
		printf(cipherText.c_str());
	}

	// the key of Caesar or Rail-fence is not an integer
	catch (const invalid_argument& e) { puts(KEY_NOT_INTEGER_ERROR_MSG), exit(EXIT_FAILURE); }

	// the key of Caesar or Rail-fence is out of range of Int
	catch (const out_of_range& e) { puts(KEY_OUT_OF_RANGE_ERROR_MSG), exit(EXIT_FAILURE); }

	// the key of Row must be made of digits (0-9) only
	catch (row_transposition_cipher_key_with_nondigits& e) { puts(ROW_TRANSPOSITION_CIPHER_KEY_WITH_NONDIGITS_ERROR_MSG), exit(EXIT_FAILURE); }

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
	int keyLength = key.length();
	int keyLengthWithoutDuplicates = 0;
	int textLength = plainText.length();
	char matrix[5][5];
	set<char> usedAlphaSet;
	string ret = "";

	// make sure that the key is made up of uppercases only
	to_uppercase(key);

	// fill in the matrix with the key
	for (int k = 0; k < keyLength; ++k) {
		char keyChar = key[k];
		int rowIdx = keyLengthWithoutDuplicates / 5;
		int colIdx = keyLengthWithoutDuplicates % 5;

		if (!usedAlphaSet.count(keyChar) && is_upper_case(keyChar)) {
			matrix[rowIdx][colIdx] = keyChar;
			usedAlphaSet.insert(keyChar);
			++keyLengthWithoutDuplicates;

			// 'I' and 'J' are viewed as the same character
			if (keyChar == 'I')
				usedAlphaSet.insert('J');
			if (keyChar == 'J')
				usedAlphaSet.insert('I');
		}
	}

	// fill in the matrix with the rest alphabets
	for (char alpha = 65, k = keyLengthWithoutDuplicates; alpha <= 90; ++alpha) {
		if (!usedAlphaSet.count(alpha)) {
			int rowIdx = k / 5;
			int colIdx = k % 5;

			++k;
			matrix[rowIdx][colIdx] = alpha;
			usedAlphaSet.insert(alpha);

			// skip 'J' since it's at the same location with 'I'
			if (alpha == 'I') ++alpha;
		}
	}

	// do ciphering
	for (int k = 0; k < textLength; k += 2) {
		char plainChar_1 = toupper(plainText[k]);
		char plainChar_2 = k + 1 == textLength ? 'X' : toupper(plainText[k + 1]);
		int rowIdx_1 = -1, colIdx_1 = -1,
			rowIdx_2 = -1, colIdx_2 = -1;

		// if a pair is a repeated letter, insert filler like 'X'
		if (plainChar_1 == plainChar_2)
			plainChar_2 = 'X', --k;

		// find 2 locations of 2 plain characters respectively
		for (int i = 0; i < 5; ++i) {
			for (int j = 0; j < 5; ++j) {
				if (matrix[i][j] == plainChar_1)
					rowIdx_1 = i, colIdx_1 = j;
				if (matrix[i][j] == plainChar_2)
					rowIdx_2 = i, colIdx_2 = j;
			}
		}
		if (rowIdx_1 == -1) {
			plainChar_1 = plainChar_1 == 'I' ? 'J' : 'I';
			for (int i = 0; i < 5; ++i)
				for (int j = 0; j < 5; ++j)
					if (matrix[i][j] == plainChar_1) {
						rowIdx_1 = i, colIdx_1 = j;
						break;
					}
		}
		if (rowIdx_2 == -1) {
			plainChar_2 = plainChar_2 == 'I' ? 'J' : 'I';
			for (int i = 0; i < 5; ++i)
				for (int j = 0; j < 5; ++j)
					if (matrix[i][j] == plainChar_2) {
						rowIdx_2 = i, colIdx_2 = j;
						break;
					}
		}

		// at the same row
		if (rowIdx_1 == rowIdx_2) {
			ret += matrix[rowIdx_1][colIdx_1 + 1 == 5 ? 0 : colIdx_1 + 1];
			ret += matrix[rowIdx_1][colIdx_2 + 1 == 5 ? 0 : colIdx_2 + 1];
		}
		// at the same col
		else if (colIdx_1 == colIdx_2) {
			ret += matrix[rowIdx_1 + 1 == 5 ? 0 : rowIdx_1 + 1][colIdx_1];
			ret += matrix[rowIdx_2 + 1 == 5 ? 0 : rowIdx_2 + 1][colIdx_1];
		}
		// not at the same row & col
		else {
			ret += matrix[rowIdx_1][colIdx_2];
			ret += matrix[rowIdx_2][colIdx_1];
		}
	}

	return ret;
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
		ret += xorred + 65;

		++keyIdx;
	}

	return ret;
}

string row(string key, string plainText) throw (row_transposition_cipher_key_with_nondigits) {
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
			throw row_transposition_cipher_key_with_nondigits();
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