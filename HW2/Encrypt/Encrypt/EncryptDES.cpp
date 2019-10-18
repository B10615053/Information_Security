#include <iostream>
#include <iomanip>
#include <string>
// 63(MSB) ~ 0(LSB)
#include <bitset>
using namespace std;

//#define DEBUG

#define RELEASE

// Feistel Network tables
const int IP[8][8] = { {58, 50, 42, 34, 26, 18, 10, 2},
					   {60, 52, 44, 36, 28, 20, 12, 4},
					   {62, 54, 46, 38, 30, 22, 14, 6},
					   {64, 56, 48, 40, 32, 24, 16, 8},
					   {57, 49, 41, 33, 25, 17,  9, 1},
					   {59, 51, 43, 35, 27, 19, 11, 3},
					   {61, 53, 45, 37, 29, 21, 13, 5},
					   {63, 55, 47, 39, 31, 23, 15, 7} };

const int IPInverse[8][8] = { {40, 8, 48, 16, 56, 24, 64, 32},
							  {39, 7, 47, 15, 55, 23, 63, 31},
							  {38, 6, 46, 14, 54, 22, 62, 30},
							  {37, 5, 45, 13, 53, 21, 61, 29},
							  {36, 4, 44, 12, 52, 20, 60, 28},
							  {35, 3, 43, 11, 51, 19, 59, 27},
							  {34, 2, 42, 10, 50, 18, 58, 26},
							  {33, 1, 41,  9, 49, 17, 57, 25} };

// f-Function tables
const int E[8][6] = { {32,  1,  2,  3,  4,  5},
					  { 4,  5,  6,  7,  8,  9},
					  { 8,  9, 10, 11, 12, 13},
					  {12, 13, 14, 15, 16, 17},
					  {16, 17, 18, 19, 20, 21},
					  {20, 21, 22, 23, 24, 25},
					  {24, 25, 26, 27, 28, 29},
					  {28, 29, 30, 31, 32,  1} };

const int S[8][4][16] = {   // S1
							{ {14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7},
							  { 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8},
							  { 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0},
							  {15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13} },
							// S2
							{ {15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10},
							  { 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5},
							  { 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15},
							  {13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9} },
							// S3
							{ {10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8},
							  {13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1},
							  {13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7},
							  { 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12} },
							// S4
							{ { 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15},
							  {13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9},
							  {10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4},
							  { 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14} },
							// S5
							{ { 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9},
							  {14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6},
							  { 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14},
							  {11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3} },
							// S6
							{ {12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11},
							  {10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8},
							  { 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6},
							  { 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13} },
							// S7
							{ { 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1},
							  {13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6},
							  { 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2},
							  { 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12} },
							// S8
							{ {13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7},
							  { 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2},
							  { 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8},
							  { 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11} }
};

const int P[4][8] = { {16,  7, 20, 21, 29, 12, 28, 17},
					  { 1, 15, 23, 26,  5, 18, 31, 10},
					  { 2,  8, 24, 14, 32, 27,  3,  9},
					  {19, 13, 30,  6, 22, 11,  4, 25} };

// Key Schedule tables
const int PCInverse1[8][7] = { {57, 49, 41, 33, 25, 17,  9},
							   { 1, 58, 50, 42, 34, 26, 18},
							   {10,  2, 59, 51, 43, 35, 27},
							   {19, 11,  3, 60, 52, 44, 36},
							   {63, 55, 47, 39, 31, 23, 15},
							   { 7, 62, 54, 46, 38, 30, 22},
							   {14,  6, 61, 53, 45, 37, 29},
							   {21, 13,  5, 28, 20, 12,  4} };

const int PCInverse2[8][6] = { {14, 17, 11, 24,  1,  5},
							   { 3, 28, 15,  6, 21, 10},
							   {23, 19, 12,  4, 26,  8},
							   {16,  7, 27, 20, 13,  2},
							   {41, 52, 31, 37, 47, 55},
							   {30, 40, 51, 45, 33, 48},
							   {44, 49, 39, 56, 34, 53},
							   {46, 42, 50, 36, 29, 32} };

// Release mode
#ifdef RELEASE
int main(int argc, char* argv[])
{
	string key = argv[1],
		plaintext = argv[2];
#endif // RELEASE

// Debug mode
#ifdef DEBUG
int main()
{
	string key = "",
		plaintext = "";
	cout << "###Encryter###\n" << "Instructions: \<KEY\> \<plain text\>\n";
	while (cin >> key >> plaintext)
	{
#endif // DEBUG

	bitset<64> keyInBit(stoull(key, NULL, 16)),
		plaintextInBit(stoull(plaintext, NULL, 16));
	int counter = 0;

#ifdef DEBUG
	cout << "Key in binary:";
	for (int i = 63; i >= 0; i--)
	{
		if (i % 4 == 3)
			cout << " ";
		cout << keyInBit[i];
	}
	cout << endl;
#endif // DEBUG key in bits

	// Initial key parts PC-1
	bitset<28> leftKey(0),
		rightKey(0);
	counter = 55;
	for (int r = 0; r < 8; r++)
	{
		for (int c = 0; c < 7; c++)
		{
			if (keyInBit[64 - PCInverse1[r][c]])
			{
				// Fill into left part(higher bit)
				if (counter >= 28)
					leftKey.set(counter  - 28);
				// Fill into right part(Lower bit)
				else
					rightKey.set(counter);
			}
			counter--;
		}
	}

#ifdef DEBUG
	cout << "Key left part 0\'s:";
	for (int l = 27; l >= 0; l--)
	{
		if (l % 4 == 3)
			cout << " ";
		cout << leftKey[l];
	}
	cout << endl;
	cout << "Key right part 0\'s:";
	for (int r = 27; r >= 0; r--)
	{
		if (r % 4 == 3)
			cout << " ";
		cout << rightKey[r];
	}
	cout << endl;
	cout << endl;
#endif // DEBUG left part and right part of round key(56)

#ifdef DEBUG
	cout << "Plain text in binary:";
	for (int i = 63; i >= 0; i--)
	{
		if (i % 4 == 3)
			cout << " ";
		cout << plaintextInBit[i];
	}
	cout << endl;
	cout << endl;
#endif // DEBUG plain text in bits
	
	// Initial permutation
	bitset<32> left(0),
		right(0);
	counter = 63;
	for (int r = 0; r < 8; r++)
	{
		for (int c = 0; c < 8; c++)
		{
			if (plaintextInBit[64 - IP[r][c]])
			{
				// Fill into left part(higher bit)
				if (counter >= 32)
					left.set(counter - 32);
				// Fill into right part(Lower bit)
				else
					right.set(counter);
			}
			counter--;
		}
	}

	// 16 rounds
	for (int round = 1; round <= 16; round++)
	{	
		// Key schedule
		bitset<48> roundKey(0);
		// Left Shift
		// Rotate one time
		if (round == 1 || round == 2 || round == 9 || round == 16)
		{
			bitset<1> temp(0);
			// Rotate left part of key
			temp[0] = leftKey[27];
			leftKey <<= 1;
			leftKey[0] = temp[0];
			// Rotate right part of key
			temp[0] = rightKey[27];
			rightKey <<= 1;
			rightKey[0] = temp[0];
		}
		// Rotate two times
		else
		{
			bitset<2> temp(0);
			// Rotate left part of key
			temp[1] = leftKey[27];
			temp[0] = leftKey[26];
			leftKey <<= 2;
			leftKey[1] = temp[1];
			leftKey[0] = temp[0];
			// Rotate right part of key
			temp[1] = rightKey[27];
			temp[0] = rightKey[26];
			rightKey <<= 2;
			rightKey[1] = temp[1];
			rightKey[0] = temp[0];
		}
		// Create round key PC-2
		counter = 47;
		for (int r = 0; r < 8; r++)
		{
			for (int c = 0; c < 6; c++)
			{
				
				// Get bit from left part(higher bit)
				if (PCInverse2[r][c] >= 29)
				{
					if (rightKey[27 - (PCInverse2[r][c] - 28 - 1)])
						roundKey.set(counter);
				}
				// Get bit from right part(lower bit)
				else
				{
					if (leftKey[27 - (PCInverse2[r][c] - 1)])
						roundKey.set(counter);
				}
				counter--;
			}
		}

#ifdef DEBUG
		cout << "Key left part " << round << "\'s:";
		for (int l = 27; l >= 0; l--)
		{
			if (l % 4 == 3)
				cout << " ";
			cout << leftKey[l];
		}
		cout << endl;
		cout << "Key right part " << round << "\'s:";
		for (int r = 27; r >= 0; r--)
		{
			if (r % 4 == 3)
				cout << " ";
			cout << rightKey[r];
		}
		cout << endl;
		cout << "Round" << round << "\'s Key:";
		for (int l = 47; l >= 0; l--)
		{
			if (l % 4 == 3)
				cout << " ";
			cout << roundKey[l];
		}
		cout << endl;
#endif // DEBUG left part and right part of round key(56)

#ifdef DEBUG
		cout << "Left part " << round - 1 << "\'s:";
		for (int l = 31; l >= 0; l--)
		{
			if (l % 4 == 3)
				cout << " ";
			cout << left[l];
		}
		cout << endl;
		cout << "Right part " << round - 1 << "\'s:";
		for (int r = 31; r >= 0; r--)
		{
			if (r % 4 == 3)
				cout << " ";
			cout << right[r];
		}
		cout << endl;
#endif // DEBUG left part and right part of plaintext(64)

		// f-Function
		bitset<48> expansion(0);
		bitset<32> expansionAfterSBox(0),
			rightAfterF(0);
		// Expansion the right part
		counter = 47;
		for (int r = 0; r < 8; r++)
		{
			for (int c = 0; c < 6; c++)
			{
				if (right[32 - E[r][c]])
					expansion.set(counter);
				counter--;
			}
		}
		// XOR of expansion and round key
		expansion ^= roundKey;
		// Run 8 S-boxes
		counter = 31;
		for (int s = 8; s >= 1; s--)
		{
			int row = expansion[s * 6 - 1] * 2 + expansion[s * 6 - 6],
				column = expansion[s * 6 - 2] * 8 + expansion[s * 6 - 3] * 4 + expansion[s * 6 - 4] * 2 + expansion[s * 6 - 5],
				sContext = 0;
			sContext = S[8 - s][row][column];
			bitset<4> temp(sContext);
			for (int c = 3; c >= 0; c--)
			{
				expansionAfterSBox[counter] = temp[c];
				counter--;
			}
		}
		// Permutation
		counter = 31;
		for (int r = 0; r < 4; r++)
		{
			for (int c = 0; c < 8; c++)
			{
				if (expansionAfterSBox[32 - P[r][c]])
					rightAfterF.set(counter);
				counter--;
			}
		}

#ifdef DEBUG
		cout << "f-Function\n";
		cout << "Expansion:";
		for (int e = 47; e >= 0; e--)
		{
			if (e % 4 == 3)
				cout << " ";
			cout << expansion[e];
		}
		cout << endl;
		cout << "After XOR with round key:";
		for (int e = 47; e >= 0; e--)
		{
			if (e % 4 == 3)
				cout << " ";
			cout << expansion[e];
		}
		cout << endl;
		cout << "After 8 S-Box:";
		for (int s = 31; s >= 0; s--)
		{
			if (s % 4 == 3)
				cout << " ";
			cout << expansionAfterSBox[s];
		}
		cout << endl;
		cout << "After Permutation:";
		for (int f = 31; f >= 0; f--)
		{
			if (f % 4 == 3)
				cout << " ";
			cout << rightAfterF[f];
		}
		cout << endl;
#endif // DEBUG f-Function

		// XOR
		bitset<32> leftAfterXOR(0);
		leftAfterXOR = left ^ rightAfterF;

#ifdef DEBUG
		cout << "XOR\n";
		cout << "Left part after XOR with f-function:";
		for (int l = 31; l >= 0; l--)
		{
			if (l % 4 == 3)
				cout << " ";
			cout << leftAfterXOR[l];
		}
		cout << endl;
		cout << endl;
#endif // DEBUG XOR

		// Exchange between left part and right part
		left = right;
		right = leftAfterXOR;
	}

#ifdef DEBUG
	cout << "Left part 16\'s:";
	for (int l = 31; l >= 0; l--)
	{
		if (l % 4 == 3)
			cout << " ";
		cout << left[l];
	}
	cout << endl;
	cout << "Right part 16\'s:";
	for (int r = 31; r >= 0; r--)
	{
		if (r % 4 == 3)
			cout << " ";
		cout << right[r];
	}
	cout << endl;
	cout << endl;
#endif // DEBUG

	// Final permutation(NEED Exchange)
	bitset<64> ciphertextInBit(0);
	counter = 63;
	for (int r = 0; r < 8; r++)
	{
		for (int c = 0; c < 8; c++)
		{
			// Get bit from left part(lower bit)
			if (IPInverse[r][c] >= 33)
			{
				if (left[31 - (IPInverse[r][c] - 32 - 1)])
					ciphertextInBit.set(counter);
			}
			// Get bit from right part(higher bit)
			else
			{
				if (right[31 - (IPInverse[r][c] - 1)])
					ciphertextInBit.set(counter);
			}
			counter--;
		}
	}

// Debug mode
#ifdef DEBUG
		cout << "0x" << std::setfill('0') << std::setw(16) << std::hex << uppercase << ciphertextInBit.to_ullong() << endl;
		cin.clear();
	}
#endif // DEBUG


// Release mode
#ifdef RELEASE
	cout << "0x" << std::setfill('0') << std::setw(16) << std::hex << uppercase << ciphertextInBit.to_ullong();
#endif // RELEASE

	return 0;
}
