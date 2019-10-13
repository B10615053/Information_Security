#pragma once

#include "Utility.h"
//#define BITSET_DEBUG

// class definition
template <int L> class Bitset;

// functions which are not directly related to the 'Bitset' class
string hexChar2BinaryString(char);
vector<Bitset<48> > generateAllSubKeys(Bitset<64>&);
Bitset<64> doDesDecryption(Bitset<64>&, vector<Bitset<48> >&);
Bitset<32> f(Bitset<32>&, Bitset<48>&);
Bitset<4> sBoxSubstitution(Bitset<6>&, int);

// class implementation, L stands for length
template <int L> class Bitset {
public:
	Bitset<L>();
	Bitset<L>(string, bool);
	Bitset<L>(unsigned);

	void set(int, int);

	// O stands for output-length
	template <int O> Bitset<O> permute(const int*);
	template <int O> Bitset<O> unpermute(const int*);

	// S stands for sub-length
	template <int S> Bitset<S> getSubRange(int);

	Bitset<L> copy();

	unsigned toUnsignedInt();
	string toString();
	string toHexString(bool);

	Bitset<L> operator<<(int);
	Bitset<L> operator>>(int);

	int operator[](int);

	Bitset<L> operator&(const Bitset<L>&);
	Bitset<L> operator|(const Bitset<L>&);
	Bitset<L> operator^(const Bitset<L>&);
	Bitset<L> operator~();

	// A and B are pronouns of lhs and rhs, since L already has its meaning (length)
	template <int A, int B> friend Bitset<A + B> operator+(const Bitset<A>&, const Bitset<B>&);

	void formattedPrintOut(string, string, int, bool showHalfHint = false);

private:
	string bits;
};

#pragma region class methods implementations

// default constructor
template <int L> Bitset<L>::Bitset() {
	bits = "";
	for (int k = 0; k < L; ++k)
		bits += "0";
}

// constructor with a binary-string or a hex-string
template <int L> Bitset<L>::Bitset(string pString, bool isBinaryString) {
	// input-string is a binary-string, e.g. "1010...01"
	if (isBinaryString)
		bits = pString;

	// input-string is a hex-string, e.g. "0xAD94...2F8"
	else {
		// remove the prefix of '0x' if exists
		string hexStr = pString;
		if (pString.length() >= 2 && pString[0] == '0' && (pString[1] == 'x' || pString[1] == 'X'))
			hexStr = pString.substr(2);

		// build the buffer for the bitset
		bits = "";
		for (int k = 0; k < 16; ++k)
			bits += hexChar2BinaryString(hexStr[k]);
	}
}

// constructor with an unsigned integer
template <int L> Bitset<L>::Bitset(unsigned integer) {
	Bitset<L> bitsetBuffer;
	int bufIdx = L - 1;

	while (integer > 0 && bufIdx >= 0) {
		int remainder = integer % 2;
		integer >>= 1;

		bitsetBuffer.set(bufIdx, remainder);
		--bufIdx;
	}

	bits = bitsetBuffer.toString();
}

// set a certain value
template <int L> void Bitset<L>::set(int idx, int value) {
	bits[idx] = value >= 48 && value <= 57 ? value : value + 48;
}

// do the permutation (O stands for output-length)
template <int L> template <int O> Bitset<O> Bitset<L>::permute(const int* table) {
	string buffer = "";
	for (int k = 0; k < O; ++k)
		buffer += bits[table[k] - 1];
	return Bitset<O>(buffer, true);
}

// do the un-permutation
template <int L> template <int O> Bitset<O> Bitset<L>::unpermute(const int* table) {
	string buffer = bits;
	for (int k = 0; k < O; ++k)
		buffer[table[k] - 1] = bits[k];
	return Bitset<O>(buffer, true);
}

// get the sub range of the bitset (S stands for sub-length)
template <int L> template<int S> Bitset<S> Bitset<L>::getSubRange(int startIndex) {
	Bitset<S> ret;
	for (int k = 0; k < S && k + startIndex < L; ++k)
		ret.set(k, this->bits[k + startIndex]);
	return ret;
}

// copy this bitset
template <int L> Bitset<L> Bitset<L>::copy() {
	Bitset<L> ret;
	for (int k = 0; k < L; ++k)
		ret.set(k, this->bits[k]);
	return ret;
}

// convert the bitset into an unsigned integer value
template <int L> unsigned Bitset<L>::toUnsignedInt() {
	unsigned ret = 0, multiplier = 1;
	for (int k = L - 1; k >= 0; --k)
		ret += unsigned(this->bits[k] - 48) * multiplier,
		multiplier <<= 1;
	return ret;
}

// convert the bitset into a string
template <int L> string Bitset<L>::toString() {
	return this->bits;
}

// convert the bitset into a hex string
template <int L> string Bitset<L>::toHexString(bool withPrefix) {
	string ret = withPrefix ? "0x" : "";

	for (int k = 0; k < L; k += 4) {
		int number = int(
			(this->bits[  k  ] - 48) * 8 +
			(this->bits[k + 1] - 48) * 4 +
			(this->bits[k + 2] - 48) * 2 +
			(this->bits[k + 3] - 48) * 1
		);
		ret += number + (number >= 0 && number <= 9 ? 48 : 97 - 10);
	}

	return ret;
}

// shift left
template <int L> Bitset<L> Bitset<L>::operator<<(int shiftOffset) {
	bitset<L> temp = bitset<L>(this->bits) << shiftOffset;
	return Bitset<L>(temp.to_string(), true);
}

// shift right
template <int L> Bitset<L> Bitset<L>::operator>>(int shiftOffset) {
	bitset<L> temp = bitset<L>(this->bits) >> shiftOffset;
	return Bitset<L>(temp.to_string(), true);
}

// get a bit by an index
template <int L> int Bitset<L>::operator[](int idx) {
	return this->bits[idx] - 48;
}

// 'and' operation
template <int L> Bitset<L> Bitset<L>::operator&(const Bitset<L>& lhs) {
	string buffer = "";
	for (int k = 0; k < L; ++k) {
		int newBit = ((this->bits[k] - 48) & (lhs.bits[k] - 48));
		buffer += (newBit + 48);
	}
	return Bitset<L>(buffer, true);
}

// 'or' operation
template <int L> Bitset<L> Bitset<L>::operator|(const Bitset<L>& lhs) {
	string buffer = "";
	for (int k = 0; k < L; ++k) {
		int newBit = ((this->bits[k] - 48) | (lhs.bits[k] - 48));
		buffer += (newBit + 48);
	}
	return Bitset<L>(buffer, true);
}

// 'xor' operation
template <int L> Bitset<L> Bitset<L>::operator^(const Bitset<L>& lhs) {
	string buffer = "";
	for (int k = 0; k < L; ++k) {
		int newBit = ((this->bits[k] - 48) ^ (lhs.bits[k] - 48));
		buffer += (newBit + 48);
	}
	return Bitset<L>(buffer, true);
}

// 'not' operation
template <int L> Bitset<L> Bitset<L>::operator~() {
	string buffer = "";
	for (int k = 0; k < L; ++k) {
		int bit = this->bits[k] - 48;
		buffer += bit == 0 ? "1" : "0";
	}
	return Bitset<L>(buffer, true);
}

// concatenate two bitsets
template <int A, int B> Bitset<A + B> operator+(const Bitset<A>& lhs, const Bitset<B>& rhs) {
	Bitset<A + B> ret;
	for (int k = 0; k < A + B; ++k)
		ret.set(k, k < A ? lhs.bits[k] : rhs.bits[k - A]);
	return ret;
}

// for debugging, print out the content of the bitset with a certain format
template <int L> void Bitset<L>::formattedPrintOut(string TAG, string separator, int unitLength, bool showHalfHint) {
	std::cout << TAG << ": ";
	for (int k = 0; k < this->bits.length(); ++k) {
		if (k != 0 && k % unitLength == 0)
			std::cout << separator;
		if (showHalfHint && this->bits.length() % 2 == 0 && k == this->bits.length() / 2)
			std::cout << "+ ";
		std::cout << this->bits[k];
	}
	std::cout << std::endl;
}

#pragma endregion