#include "Bitset.h"

// convert a character into a binary string
string hexChar2BinaryString(char hexChar) {
	int number = hexChar;
	char ret[5];

	// convert the character into the corresponding number
	if (hexChar >= 97 && hexChar <= 122) number -= 97 - 10;
	else if (hexChar >= 65 && hexChar <= 90) number -= 65 - 10;
	else number -= 48;

	// build the string
	for (int k = 3; k >= 0; --k)
		ret[k] = (number % 2) + 48,
		number /= 2;
	ret[4] = 0;

	return string(ret);
}

// generate sub-keys of all 16 rounds
vector<Bitset<48> > generateAllSubKeys(Bitset<64>& keyCode) {
	Bitset<56> roundKeyCode = keyCode.permute<56>(PC_1);
	vector<Bitset<48> > ret;

	for (int round = 1; round <= 16; ++round) {
		Bitset<28> left, right;

		// split the round-key into two halves
		for (int k = 0; k < 56; ++k)
			if (k < 28)
				left.set(k, roundKeyCode[k]);
			else
				right.set(k - 28, roundKeyCode[k]);

		// round 1, 2, 9, 16, -> rotate left by 1 bit
		if (round == 1 || round == 2 || round == 9 || round == 16)
			left = ((left << 1) | (left >> 27)),
			right = ((right << 1) | (right >> 27));
		// the rest rounds, -> rotate left by 2 bits
		else
			left = ((left << 2) | (left >> 26)),
			right = ((right << 2) | (right >> 26));

		// update the round-key
		for (int k = 0; k < 56; ++k)
			if (k < 28)
				roundKeyCode.set(k, left[k]);
			else
				roundKeyCode.set(k, right[k - 28]);

		// permute the round-key through pc-2 and put it into the return-vector
		Bitset<48> subKeyCode = roundKeyCode.permute<48>(PC_2);

		ret.push_back(subKeyCode);
	}

	return ret;
}

// do the decryption of des
Bitset<64> doDesDecryption(Bitset<64>& cipherCode, vector<Bitset<48> >& subKeyList) {
	/// Step 1
	// unpermute through fp (inverse of ip)
	Bitset<64> unpermutedCipherCode = cipherCode.unpermute<64>(IP_inverse);
	Bitset<64> roundCode = unpermutedCipherCode.getSubRange<32>(32) + unpermutedCipherCode.getSubRange<32>(0);

	#ifdef BITSET_DEBUG
	cipherCode.formattedPrintOut("     input", " ", 4, true);
	unpermutedCipherCode.formattedPrintOut("unpermuted", " ", 4, true);
	#endif

	/// Step 2
	// do 16-round feistel networks in the reversed order
	for (int round = 15; round >= 0; --round) {

		/// Step 2.1
		// swap between the left- and right- parts
		Bitset<32> swappedLeft = roundCode.getSubRange<32>(32);
		Bitset<32> swappedRight = roundCode.getSubRange<32>(0);

		/// Step 2.2
		// do f-function with the parameters of swapped-right and round-key
		Bitset<32> f_functioned = f(swappedRight, subKeyList[round]);

		/// Step 2.3
		// do xor with swapped-left and f-functioned value
		swappedLeft = (swappedLeft ^ f_functioned);

		/// Step 2.4
		// re-build the round-code
		roundCode = swappedLeft + swappedRight;
	}

	/// Step 3
	// unpermute through ip
	Bitset<64> unpermutedPlainCode = roundCode.unpermute<64>(IP);

	return unpermutedPlainCode;
}

// f-function of des
Bitset<32> f(Bitset<32>& rightCode, Bitset<48>& roundKeyCode) {
	/// Step 1
	// permute with E
	Bitset<48> expanded = rightCode.permute<48>(F_function_E);

	#ifdef BITSET_DEBUG
	rightCode.formattedPrintOut("f-input   ", " ", 4);
	expanded.formattedPrintOut("f-expanded", " ", 4);
	#endif

	/// Step 2
	// do xor with sub-key of this round
	Bitset<48> xored = expanded ^ roundKeyCode;

	#ifdef BITSET_DEBUG
	roundKeyCode.formattedPrintOut("round--key", " ", 4);
	xored.formattedPrintOut("f -- xored", " ", 4);
	#endif

	/// Step 3
	// do eight s-box's
	string sBoxedBuffer = "";
	for (int k = 0, s_idx = 0; k < 48; k += 6, ++s_idx) {
		Bitset<6> subset = xored.getSubRange<6>(k);
		Bitset<4> s_boxed = sBoxSubstitution(subset, s_idx);

		sBoxedBuffer += s_boxed.toString();
	}
	Bitset<32> sBoxed(sBoxedBuffer, true);

	/// Step 4
	// permute with P
	Bitset<32> ret = sBoxed.permute<32>(F_function_P);

	return ret;
}

// do s-box substitution
Bitset<4> sBoxSubstitution(Bitset<6>& inputCode, int sIdx) {
	int rowIdx = (((inputCode[0] << 1) & 2) | (inputCode[5] & 1));
	int colIdx = int(((inputCode & Bitset<6>("011110", true)) >> 1).toUnsignedInt());
	int position = rowIdx * 16 + colIdx;

	#ifdef BITSET_DEBUG
	inputCode.formattedPrintOut("\ns-box-inp", " ", 1000);
	std::cout << "s_idx -> " << sIdx << std::endl;

	for (int k = 0; k < 16; ++k)
		printf("%2d ", k);
	for (int k = 0; k < 64; ++k) {
		if (k % 16 == 0)
			puts("");
		printf("%02d ", F_function_S_boxes[sIdx][k]);
	}
	puts("\n");

	std::cout << "row -> " << rowIdx << std::endl;
	std::cout << "col -> " << colIdx << std::endl;
	std::cout << "val -> " << F_function_S_boxes[sIdx][position] << std::endl << std::endl;
	#endif

	return Bitset<4>(unsigned(F_function_S_boxes[sIdx][position]));
}