#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <cmath>
#include <vector>
using namespace std;

//#define DEBUG

#define RELEASE

bool stringToInteger(int&, string&);
bool stringToDigit(vector<int>&, string&, bool&);
string caesar(int&, string&);
string playfair(string&, string&);
string vernam(string&, string&);
string rowTransposition(vector<int>&, string&, bool&);
string railFence(int&, string&);

#ifdef RELEASE

int main(int argc, char* argv[])
{
	string cipher = argv[1],
		key = argv[2],
		ciphertext = argv[3],
		plaintext = "";

#endif // RELEASE

// Debug mode
#ifdef DEBUG
int main()
{
	string cipher = "",
		key = "",
		ciphertext = "",
		plaintext = "";
	cout << "###Decrypter###\n" << "Instructions: \<cipher\> \<KEY\> \<CIPHER TEXT\>\n";
	while (cin >> cipher >> key >> ciphertext)
	{
#endif // DEBUG

	// Transform all alphabets in cipher type to lower case
	transform(cipher.begin(), cipher.end(), cipher.begin(), tolower);
	// Transform all alphabets in cipher text to upper case
	transform(ciphertext.begin(), ciphertext.end(), ciphertext.begin(), toupper);
	// Check type of cipher
	if (cipher == "caesar")
	{
		int integerKey = 0;
		bool isNumber = stringToInteger(integerKey, key);
		if (isNumber)
			plaintext = caesar(integerKey, ciphertext);
		else
		{
			cout << "WARNING!!!\n"
				<< "The key of Caesar Cipher must be an integer.\n";
		}
	}
	else if (cipher == "playfair")
	{
		transform(key.begin(), key.end(), key.begin(), toupper);
		plaintext = playfair(key, ciphertext);
	}
	else if (cipher == "vernam")
	{
		transform(key.begin(), key.end(), key.begin(), toupper);
		plaintext = vernam(key, ciphertext);
	}
	else if (cipher == "row")
	{
		vector<int> digitKey;
		bool haveZeroKey = false;
		bool isNumber = stringToDigit(digitKey, key, haveZeroKey);
		if (isNumber)
			plaintext = rowTransposition(digitKey, ciphertext, haveZeroKey);
		else
		{
			cout << "WARNING!!!\n"
				<< "The key of Row Transposition Cipher must be made of digits (0-9) only.\n";
		}
	}
	else if (cipher == "rail_fence")
	{
		int integerKey = 0;
		bool isNumber = stringToInteger(integerKey, key);
		if (isNumber)
			plaintext = railFence(integerKey, ciphertext);
		else
		{
			cout << "WARNING!!!\n"
				<< "The key of Rail Fence Cipher must be an integer.\n";
		}
	}
	else
	{
		cout << "WARNING!!!\n" 
			<< "No such cipher method exists.\n"
			<< "Only caesar, playfair, vernam, row, and rail_fence allowed.\n";
	}

// Debug mode
#ifdef DEBUG
		cout << plaintext << endl;
		cin.clear();
	}
#endif // DEBUG

// Release mode
#ifdef RELEASE
	cout << plaintext;
#endif // RELEASE

	return 0;
}

// Transfer string to integer and check whether is number
bool stringToInteger(int& integerKey, string& key)
{
	bool isNumber = true;
	if (key[0] == '-' || key[0] == '+')
	{
		if (key.length() > 1)
		{
			for (int l = key.length() - 1, p = 0; l >= 1; l--, p++)
			{
				int temp = key[l] - (int)'0';
				if (temp >= 0 && temp <= 9)
					integerKey += (temp * pow(10, p));
				else
				{
					isNumber = false;
					break;
				}
			}
			if (key[0] == '-')
				integerKey *= -1;
		}
	}
	else
	{
		for (int l = key.length() - 1, p = 0; l >= 0; l--, p++)
		{
			int temp = key[l] - (int)'0';
			if (temp >= 0 && temp <= 9)
				integerKey += (temp * pow(10, p));
			else
			{
				isNumber = false;
				break;
			}
		}
	}
	return isNumber;
}
// Transfer string to a digit and check whether is number
bool stringToDigit(vector<int>& digitKey, string& key, bool& haveZeroKey)
{
	bool isNumber = true;
	for (int l = 0; l < key.length(); l++)
	{
		int temp = key[l] - (int)'0';
		if (temp == 0)
			haveZeroKey = true;
		if (temp >= 0 && temp <= 9)
			digitKey.push_back(temp);
		else
		{
			isNumber = false;
			break;
		}
	}
	return isNumber;
}
// Caesar Cipher
string caesar(int& key, string& ciphertext)
{
	string plaintext = "";
	key %= 26;
	for (int l = 0; l < ciphertext.length(); l++)
	{
		int temp = (ciphertext[l] - (int)'A') - key;
		if (temp < 0)
			temp += 26;
		else if (temp >= 26)
			temp -= 26;
		plaintext += (char)(temp + (int)'a');
	}
	return plaintext;
}
// Playfair Cipher
string playfair(string& key, string& ciphertext)
{
	string plaintext = "";
	vector<vector<char>> keyMatrix;
	vector<pair<int, int>> alphabet;
	int counter = 0;
	bool fillKey = true;
	alphabet.assign(26, make_pair(-1,-1));
	for (int r = 0; r < 5; r++)
	{
		vector<char> temp;
		for (int c = 0; c < 5; c++)
		{
			bool findKeyAlphabet = false,
				findAlphabet = false;
			// Fill the matrix by key
			if (fillKey)
			{
				while (!findKeyAlphabet)
				{
					if (alphabet[(int)key[counter] - (int)'A'].first == -1)
					{
						// Found I then set J
						if ((int)key[counter] - (int)'A' == (int)('I' - 'A'))
						{
							alphabet[(int)key[counter] - (int)'A' + 1] = make_pair(r, c);
						}
						// Found J then set I
						else if ((int)key[counter] - (int)'A' == (int)('J' - 'A'))
						{
							alphabet[(int)key[counter] - (int)'A' - 1] = make_pair(r, c);
						}
						alphabet[(int)key[counter] - (int)'A'] = make_pair(r, c);
						temp.push_back((char)(key[counter] - (int)'A' + (int)'a'));
						findKeyAlphabet = true;
						findAlphabet = true;
					}
					else if (counter == (key.length() - 1))
					{
						findKeyAlphabet = true;
					}
					counter++;
					// Change the lebel and reset the counter
					if (counter >= key.length())
					{
						fillKey = false;
						counter = 0;
					}
				}
			}
			// Search unused alphabet form the tabel
			if (!findAlphabet)
			{
				while (!findAlphabet && (counter < 26))
				{
					if (alphabet[counter].first == -1)
					{
						// Found I then set J
						if (counter == (int)('I' - 'A'))
						{
							alphabet[counter + 1] = make_pair(r, c);
						}
						// Found J then set I
						else if (counter == (int)('J' - 'A'))
						{
							alphabet[counter - 1] = make_pair(r, c);
						}
						alphabet[counter] = make_pair(r, c);
						temp.push_back((char)(counter + 'a'));
						findAlphabet = true;
					}
					counter++;
				}
			}
		}
		keyMatrix.push_back(temp);
	}
#ifdef DEBUG
	for (int r = 0; r < 5; r++)
	{
		for (int c = 0; c < 5; c++)
		{
			if (keyMatrix[r][c] == 'i' || keyMatrix[r][c] == 'j')
				cout << " i/j";
			else
				cout << "  " << keyMatrix[r][c] << " ";
		}
		cout << endl;
	}
	for (int a = 0; a < 26; a++)
	{
		cout << (char)(a + (int)'a') << ": " << alphabet[a].first << ", " << alphabet[a].second << endl;
	}
#endif // DEBUG
	// Get the plain text
	for (int c = 0; c < ciphertext.length(); c += 2)
	{
		int one = (int)ciphertext[c] - (int)'A',
			two = (int)ciphertext[c + 1] - (int)'A';
		// Two alphabet at same row, shift left
		if (alphabet[one].first == alphabet[two].first)
		{
			// First alphabet
			if ((alphabet[one].second - 1) < 0)
				plaintext += keyMatrix[alphabet[one].first][alphabet[one].second + 4];
			else
				plaintext += keyMatrix[alphabet[one].first][alphabet[one].second - 1];
			// Second alphabet
			if ((alphabet[two].second - 1) < 0)
				plaintext += keyMatrix[alphabet[two].first][alphabet[two].second + 4];
			else
				plaintext += keyMatrix[alphabet[two].first][alphabet[two].second - 1];
		}
		// Two alphabet at same column, shift up
		else if (alphabet[one].second == alphabet[two].second)
		{
			// First alphabet
			if ((alphabet[one].first - 1) < 0)
				plaintext += keyMatrix[alphabet[one].first + 4][alphabet[one].second];
			else
				plaintext += keyMatrix[alphabet[one].first - 1][alphabet[one].second];
			// Second alphabet
			if ((alphabet[two].first - 1) < 0)
				plaintext += keyMatrix[alphabet[two].first + 4][alphabet[two].second];
			else
				plaintext += keyMatrix[alphabet[two].first - 1][alphabet[two].second];
		}
		// Two alphabet at diagonal position, get opposite column
		else
		{
			plaintext += keyMatrix[alphabet[one].first][alphabet[two].second];
			plaintext += keyMatrix[alphabet[two].first][alphabet[one].second];
		}
	}
	return plaintext;
}
// Vernam Proposed The Autokey System
string vernam(string& key, string& ciphertext)
{
	string plaintext = "";
	// Get the plain text
	for (int c = 0, k = 0; c < ciphertext.length(); c++, k++)
	{
		int cipherAlphabet = 0,
			keyAlphabet = 0,
			plainAlphabet = 0;
		cipherAlphabet = (int)ciphertext[c] - (int)'A';
		// Check whether use the Autokey Cipher
		if (k < key.length())
			keyAlphabet = (int)key[k] - (int)'A';
		else
			keyAlphabet = (int)plaintext[(k - key.length())] - (int)'a';
		plainAlphabet = cipherAlphabet ^ keyAlphabet;
#ifdef DEBUG
		cout << ciphertext[c] << " " << cipherAlphabet << " , " << (char)(keyAlphabet + (int)'A') << " " << keyAlphabet << " , " << (char)(plainAlphabet + 'a') << " " << plainAlphabet << endl;
#endif // DEBUG
		plaintext += (char)(plainAlphabet + 'a');
	}
	return plaintext;
}
// Row Transpostion
string rowTransposition(vector<int>& key, string& ciphertext, bool& haveZeroKey)
{
	string plaintext = "";
	int left = 0,
		counter = 0;
	vector<int> column,
		order;
	left = ciphertext.length() % key.size();
	order.assign(key.size(), -1);
	// Get each column's amount of alphabet
	counter = ciphertext.length() / key.size();
	for (int c = 0; c < key.size(); c++)
	{
		if (c < left)
			column.push_back(counter + 1);
		else
			column.push_back(counter);
	}
	// Get the order of the key
	for (int k = 0; k < key.size(); k++)
	{
		if (!haveZeroKey)
			order[key[k] - 1] = k;
		else
			order[key[k]] = k;
	}
	// Change the column context to the position of the cipher text
	counter = 0;
	for (int o = 0; o < order.size(); o++)
	{
		int temp = column[order[o]];
		column[order[o]] = counter;
		counter += temp;
	}
#ifdef DEBUG
	cout << "Cipher text in each column:\n";
	for (int c = 0; c < column.size(); c++)
	{
		cout << " " << column[c];
	}
	cout << endl;
#endif // DEBUG
	// Get the plain text
	counter = 0;
	while (counter < ciphertext.length())
	{
		plaintext += ciphertext[column[(counter % column.size())]];
		column[(counter % column.size())]++;
		counter++;
	}
	transform(plaintext.begin(), plaintext.end(), plaintext.begin(), tolower);
	return plaintext;
}
// Rail Fence Cipher
string railFence(int& key, string& ciphertext)
{
	string plaintext = "";
	int triangle = 0,
		left = 0,
		counter = 0,
		lebel = 0;
	vector<int> level;
	bool increasing = true;
	// Triangle is shape like mirror of NIKE mark
	triangle = ciphertext.length() / ((key - 1) * 2);
	left = ciphertext.length() % ((key - 1) * 2);
	// Initialize each level's alphabet number, from top to bottom (0 to [key - 1])
	for (int k = 0; k < key; k++)
	{
		if (k == 0 || k == key - 1)
			level.push_back(triangle);
		else
			level.push_back(triangle * 2);
	}
	// Have incomplete triangle
	for (int l = 0; l < left; l++)
	{
		if (l == 0)
			level[0]++;
		else if (l == (key - 1))
			level[key - 1]++;
		else
		{
			if (l >= key)
				level[(key - 1) - (l - (key - 1))]++;
			else
				level[l]++;
		}
	}

#ifdef DEBUG
	for (int i = 0; i < level.size(); i++)
	{
		cout << "Level" << i + 1 << " = " << level[i] << endl;
	}
#endif // DEBUG

	// Change the level context to the position of the cipher text
	for (int l = 0; l < level.size(); l++)
	{
		int temp = level[l];
		level[l] = counter;
		counter += temp;
	}
	// Get the plain text
	counter = 0;
	while (counter < ciphertext.length())
	{
		plaintext += ciphertext[level[lebel]];
		level[lebel]++;
		if (increasing)
			lebel++;
		else
			lebel--;
		if (lebel == (key - 1))
			increasing = false;
		else if (lebel == 0)
			increasing = true;
		counter++;
	}
	transform(plaintext.begin(), plaintext.end(), plaintext.begin(), tolower);
	return plaintext;
}