#pragma warning (disable:4996)

#include "Utility.h"
#include "Bitset.h"
//#define DEBUG

int main(int argc, char** argv) {
	string key, cipherText;

	// the count of arguments must be 3
	#ifndef DEBUG
	if (argc != 3)
		puts(ARGC_ERROR_MSG), exit(EXIT_FAILURE);
	key = string(argv[1]);
	cipherText = string(argv[2]);
	#endif

	#ifdef DEBUG
	std::cin >> key >> cipherText;
	#endif

	#pragma region the algorithm starts here

	/// Step 1
	// get the binary codes of key and cipher-text
	Bitset<64> keyCode(key, false);
	Bitset<64> cipherCode(cipherText, false);

	/// Step 2
	// generate all sub-keys
	vector<Bitset<48> > allSubKeys = generateAllSubKeys(keyCode);

	/// Step 3
	// do des decryption
	Bitset<64> plainCode = doDesDecryption(cipherCode, allSubKeys);

	#ifdef DEBUG
	plainCode.formattedPrintOut("plain", " ", 4);
	#endif

	/// Step 4
	// output the plain text in hex
	std::cout << plainCode.toHexString(true);

	#pragma endregion

	return 0;
}
