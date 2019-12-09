import sys
import os
import random
import math

### ============================= ###

def MILLERRABINTIMES():
    return 4

### ============================= ###

# Square and Multiply
# answer = a^d mod n, d(binary) = bit(t) ~ bit(0)
def squareMultiply(a, d, n):
    answer = a
    # get bit from t - 1 to 0 
    for i in range(len(bin(d)[2:]) - 2, -1, -1):
        answer = (answer ** 2) % n
        # bit = 1
        if (d & (1 << i)):
            answer = (answer * a) % n
    return answer

### ============================= ###

# Miller–Rabin Primality Test
def millerRabinTest(prime):
    if ((prime % 2) == 0):
        return False
    for i in range(MILLERRABINTIMES()):
        # n − 1
        minusOne = prime - 1
        d = minusOne
        # Find r such that n = 2^d * r + 1 for some r >= 1 
        while (d % 2 == 0):
            d //= 2
        # Pick a random integer a in the range [2, n − 2]
        a = random.randrange(2, minusOne - 1, 1)
        """print('n = ', prime)
        print('n - 1 = ', minusOne)
        print('d = ', d)
        print('a = ', a)"""
        # x ← a^(d * 2^?) mod n
        while (d < minusOne):
            x = squareMultiply(a, d, prime)
            """print('x = ', x)"""
            if((x == prime - 1) or (x == 1)):
                d *= 2
            else:
                return False
    return True

### ============================= ###

 # Generate number of bit's prime
def primeGenerator(numberOfBit):
    isPrime = False
    while (not isPrime):
        prime = random.getrandbits(numberOfBit)
        while (len(bin(prime)[2:]) < numberOfBit):
            prime = random.getrandbits(numberOfBit)
        # Miller–Rabin Primality Test
        isPrime = millerRabinTest(prime)
    return prime

### ============================= ###

# Euclidean Algorithm
def GCD(a, b):
    if (b == 0):
        return a
    else:
        return GCD(b, a % b)

### ============================= ###

# Extended Euclidean Algorithm
def extendedGCD(a, b):
    x, old_x = 0, 1
    y, old_y = 1, 0

    while (b != 0):
        quotient = a // b
        a, b = b, a - quotient * b
        old_x, x = x, old_x - quotient * x
        old_y, y = y, old_y - quotient * y

    return old_x

### ============================= ###

# Encryption
def encryption(plainText, e, n):
    # Convert plain text into the list of numbers
    textsInNumber = [ord(c) for c in plainText]
    # Do encrytion by each character in plain text
    cipherText = ""
    print("Each word and its correspond number after Encryption")
    for c in textsInNumber:
        numberAfterEncrypt = squareMultiply(c, e, n)
        cipherText += hex(numberAfterEncrypt)[2:] + ','
        print(chr(c))
        print(hex(numberAfterEncrypt)[2:])
    # Remove the last redundant ','
    cipherText = cipherText[0 : len(cipherText) - 1]
    """# Write file
    with open('cipherText', 'w') as fout:
        fout.write(cipherText)"""
    return cipherText

### ============================= ###

# Chinese Remainder Algorithm
def chineseRemainderTheorem(C, d, p, q):
    # m = (c ^ d) mod (p * q)
    # Split up the message M into two halves (one modulo p, and one modulo q), compute each modulo separately, and then recombine them. 
    exponentialP = d % (p - 1)
    mP = squareMultiply(C, exponentialP, p)
    exponentialQ = d % (q - 1)
    mQ = squareMultiply(C, exponentialQ, q)
    inverseQ = extendedGCD(p, q)
    if (inverseQ < 0):
        inverseQ = p + q
    h = (inverseQ * (mP - mQ)) % p
    return (mQ + h * q)

### ============================= ###

# Decrytion
def decryption(cipherText, d, p, q):
    # Convert the cipher text into the list of numbers
    numbersInCipherText = [int('0x' + numberAfterEncrypt, 16) for numberAfterEncrypt in cipherText.split(',')]
    # Do decryption by each number in cipher text
    plainText = ""
    for n in numbersInCipherText:
        numberAfterDecrypt = chineseRemainderTheorem(n, d, p, q)
        plainText += chr(numberAfterDecrypt)
    return plainText

### ============================= ###

def main():
    numberOfBit = input("Input RSA's number of bit(EX: 1024) : ")
    # Generate prime p
    print("Genrating prime, Please wait . . .")
    primeP = primeGenerator(int(numberOfBit) // 2)
    print("Prime P =\n", primeP)
    # Generate prime q
    primeQ = primeGenerator(int(numberOfBit) // 2)
    while (primeP == primeQ):
        primeQ = primeGenerator(int(numberOfBit) // 2)
    print("Prime Q =\n", primeQ)
    # Compute N = p * q
    compositeN = primeP * primeQ
    print("Composite N =\n", compositeN)
    # Compute Φ(N) = (p − 1) * (q − 1)
    phi = (primeP - 1) * (primeQ - 1)
    print("Φ(N) =\n", phi)
    # Choose an integer e such that 1 < e < Φ(N) and gcd(e, Φ(N)) = 1
    publicKey = random.randrange(2, phi, 1)
    isCoprime = GCD(publicKey, phi)
    while (isCoprime != 1):
        publicKey = random.randrange(2, phi, 1)
        isCoprime = GCD(publicKey, phi)
    print("Public Key E =\n", publicKey)
    # Compute d, the modular multiplicative inverse of e (mod Φ(N)) 
    privateKey = extendedGCD(publicKey, phi)
    if (privateKey < 0):
        privateKey = privateKey + phi
    print("Private Key D =\n", privateKey)
    print("\n")
    print("===Encryption===")
    plainText = input("Input plain text: ")
    print("Encrypting, Please wait . . .")
    cipherText = encryption(plainText, publicKey, compositeN)
    print("\n")
    print("===Decryption===")
    print("The cipher text: ", cipherText)
    print("Decryptin, Please wait . . .")
    plainText = decryption(cipherText, privateKey, primeP, primeQ)
    print("The plain text: ", plainText)

### ============================= ###

main()