#!/usr/bin/env python
# coding: utf-8

import sys
import os
from Crypto.Cipher import AES
from random import randint

### ============================= ###

# for printing the formatted exception
def print_exception(main_message, *sub_messages):
    print('\n\t>>> EXCEPTION: {}'.format(main_message))
    for msg in sub_messages:
        print('\t\t\t', msg)
    print()

### ============================= ###

# convert an integer into a binary string
# if the parameter fixed_length is not None and it's an integer,
# then, the result will be formatted with leading zeros if the length is less than fixed_length
def int_to_bin_str(num, fixed_length=None):
    converted = "{0:b}".format(num)

    # designated fixed length
    if type(fixed_length) is int:
        while len(converted) < fixed_length:
            converted = '0' + converted
    
    # should be fixed as 8's multiple length
    elif type(fixed_length) is bool and fixed_length:
        num_of_padding = 8 - (len(converted) % 8)
        while num_of_padding < 8 and num_of_padding > 0:
            converted = '0' + converted
            num_of_padding -= 1

    return converted

### ============================= ###

# convert an integer into a byte array
def int_to_byte_arr(num, fixed_length=None):
    return bin_str_to_byte_arr(int_to_bin_str(num, fixed_length))

### ============================= ###

# convert a byte array into an integer
def byte_arr_to_int(byte_arr):
    ret = 0
    it = 1
    for single_byte_int in reversed(byte_arr):
        ret += single_byte_int * it
        it *= 256
    return ret

### ============================= ###

# convert a binary string into an integer
def bin_str_to_int(bin_str):
    return int('0b' + bin_str, 2)

### ============================= ###

# convert a binary string into a byte array
def bin_str_to_byte_arr(bin_str):
    return int(bin_str, 2).to_bytes(len(bin_str) // 8, 'big')

### ============================= ###

# convert an ASCII string into an integer array
def ascii_str_to_int_arr(ascii_str):
    return [ord(x) for x in ascii_str]

### ============================= ###

# for containing some mathematical methods about number theory
class NumberTheory():
    # calculate the result of ((base ** exp) mod modulus)
    @staticmethod
    def square_and_multiply(base, exp, modulus):
        # get the binary string of the passed exponent
        exp_bin_str = int_to_bin_str(exp)
        
        # the return value, initialize to 1
        ret = 1
        
        # iterate exp binary string and calc the result
        for exp_bit in exp_bin_str:
            ret = (ret * ret) % modulus
            if exp_bit == '1':
                ret = (ret * base) % modulus
        
        return ret
    
    # get the values of k and m in the first step of miller-rabin test
    @staticmethod
    def get_k_and_m_in_miller_rabin_test(candidate):
        m = candidate - 1
        k = 0
        
        while (m & 1) == 0:
            m >>= 1
            k += 1
        
        return k, m
        
    # miller-rabin test
    # for testing if a candidate is 'probably' a prime or definitely not a prime
    # if it's probably a prime, returns true, else, returns false
    @staticmethod
    def miller_rabin_test(candidate):
        # get the values of k and m
        k, m = NumberTheory.get_k_and_m_in_miller_rabin_test(candidate)
        
        # randomly choose 'a' in [2, N - 2]
        a = randint(2, candidate - 2)
        
        # compute b = ((a ** m) mod N)
        b = NumberTheory.square_and_multiply(a, m, candidate)
        
        if b != 1 and b != candidate - 1:
            i = 1
            while i < k and b != candidate - 1:
                b = NumberTheory.square_and_multiply(b, b, candidate)
                if b == 1:
                    return False
                i += 1
            
            if b != candidate - 1:
                return False
        
        # the prime probably is a prime
        return True

    # check if a number is a prime, using miller-rabin test for the number > 5
    @staticmethod
    def check_prime(num, test_times=10):
        # 2 and 3 are primes
        if num == 2 or num == 3:
            return True
        
        # if the num is 5, miller-rabin test will return false in almost all tries
        # so, manually return true if the num is 5
        if num == 5:
            return True
        
        # an even number (!= 2) is a prime
        if (num & 1) == 0:
            return False
        
        # do many times of miller-rabin tests
        test_result_list = [None] * test_times
        for i in range(test_times):
            test_result_list[i] = NumberTheory.miller_rabin_test(num)
        
        # return true if the true-result from miller-rabin tests is more than false-result
        return test_result_list.count(True) >= test_result_list.count(False)

    # generate the random bits by using block cipher as PRNG
    @staticmethod
    def random_bits_generator(need_of_bits):
        # randomly generate an IV and a key of 128 bits (16 bytes) for AES
        key = b''
        iv = b''
        for _ in range(16):
            iv += randint(0, 255).to_bytes(1, 'little')
            key += randint(0, 255).to_bytes(1, 'little')
        
        # build an AES cryptor with ECB mode
        ecb_cryptor = AES.new(key, AES.MODE_ECB)
        
        # generator random bits by AES encrypting
        ret_bytes = b''
        while len(ret_bytes) * 8 < need_of_bits:
            encrypted_iv = ecb_cryptor.encrypt(iv)
            ret_bytes += encrypted_iv
            iv = encrypted_iv
        
        # build the result bits
        ret_bits = ''
        for single_byte in ret_bytes:
            ret_bits += int_to_bin_str(single_byte, 8)
        
        # return the result bits
        return ret_bits[0:need_of_bits]

    # generate a large prime randomly
    @staticmethod
    def generate_prime_randomly(num_of_bits):
        print('Start choosing a prime...')
        
        # the biggest possible number in decimal
        biggest = 2 ** num_of_bits
        
        # in the case of the input num-of-bits is too small,
        # that mis-choosing is possible
        while True:
            # the first bit is 1, and the last bit is 1 as well
            # then randomly generate intermediate bits
            candidate_bits = '1' + NumberTheory.random_bits_generator(num_of_bits - 2) + '1'

            # convert the bits into an integer
            candidate_int = bin_str_to_int(candidate_bits)

            while candidate_int < biggest:
                # check if the candidate is a prime or not
                if NumberTheory.check_prime(candidate_int, 1):
                    print('\tFinish choosing a prime')
                    return candidate_int

                # if the current candidate is not a prime,
                # generate the next candidate by plus 2
                candidate_int += 2

    # calculate gcd
    @staticmethod
    def get_gcd(a, b):
        while b != 0:
            a, b = b, a % b
        return a

    # calculate multiplicative inverse
    @staticmethod
    def get_multiplicative_inverse(num, modulus):
        if NumberTheory.get_gcd(num, modulus) != 1:
            return None
        
        u1, u2, u3 = 1, 0, num
        v1, v2, v3 = 0, 1, modulus

        while v3 != 0:
            q = u3 // v3
            v1, v2, v3, u1, u2, u3 = (u1 - q * v1), (u2 - q * v2), (u3 - q * v3), v1, v2, v3
        
        return u1 % modulus

    # speed-up for calculating ((base ** exp) mod n) where n = p * q where p and q are both primes
    @staticmethod
    def chinese_remainder_theorem(base, exp, p, q):
        # calculate exp_p and exp_q
        exp_p = exp % (p - 1)
        exp_q = exp % (q - 1)

        # get the multiplicative inverse of q with modulus p
        q_inv = NumberTheory.get_multiplicative_inverse(q, p)

        # divide the orginal equation into 2 equations:
        # M  ≡ base ** exp mod (p * q)
        # ->
        # m1 ≡ base ** exp_p mod p
        # m2 ≡ base ** exp_q mod q
        m_1 = NumberTheory.square_and_multiply(base, exp_p, p)
        m_2 = NumberTheory.square_and_multiply(base, exp_q, q)
        
        # find the smallest positive integer as the multiple of q
        h = (q_inv * (m_1 - m_2)) % p

        # return the calculated result
        return m_2 + (h * q)

### ============================= ###

# the RSA class
class RSA():
    # fixed length of bits of saved key elements (p, q, n, e, d)
    fixed_length_of_bits_of_elements = 8000
    p, q, n, e, d = [0] * 5

    # initialize RSA, generate and save the key
    @staticmethod
    def init_rsa(num_of_bits):
        RSA.p, RSA.q, RSA.n, RSA.e, RSA.d = RSA.generate_rsa_key(num_of_bits // 2)
        RSA.save_key()
        print('\nInitialization done')

    # do RSA key generation algorithm, it returns p, q, n, e, d
    @staticmethod
    def generate_rsa_key(num_of_bits):
        # step 1. choose 2 primes p and q
        # choose p
        p = NumberTheory.generate_prime_randomly(num_of_bits)
        # in the case of p is 0, 1, or 2, re-choose
        while p <= 2:
            p = NumberTheory.generate_prime_randomly(num_of_bits)
        # choose q
        q = NumberTheory.generate_prime_randomly(num_of_bits)
        # in the case of num_of_bits is too small, that the possibility of the equality between p and q is high,
        # or q is 0, 1, or 2, re-choose
        while q == p or q <= 2:
            q = NumberTheory.generate_prime_randomly(num_of_bits)

        # step 2. compute n = p * q
        n = p * q

        # step 3. compute phi(n) = (p - 1)(q - 1)
        phi_of_n = (p - 1) * (q - 1)
        
        # step 4. select e from [1, phi(n) - 1] such that gcd(e, phi(n)) = 1
        max_tries = 50000
        # step 4-1. try 50000 times to randomly select e
        for _ in range(max_tries):
            e = randint(1, phi_of_n - 1)
            gcd = NumberTheory.get_gcd(e, phi_of_n)
            if gcd == 1:
                break
        # step 4-2. if cannot find out valid e after 50000 tries,
        #           directly assign to (phi(n) - 1) which is definitely relatively prime to phi(n)
        if gcd > 1:
            e = phi_of_n - 1
        
        # step 5. compute d such that de ≡ 1 (mod phi(n))
        d = NumberTheory.get_multiplicative_inverse(e, phi_of_n)

        # step 6. return p, q, n, e, d
        return p, q, n, e, d

    # save the key elements (p, q, n, e, d)
    @staticmethod
    def save_key():
        with open('key', 'w+b') as fout:
            fout.write(int_to_byte_arr(RSA.p, RSA.fixed_length_of_bits_of_elements))
            fout.write(int_to_byte_arr(RSA.q, RSA.fixed_length_of_bits_of_elements))
            fout.write(int_to_byte_arr(RSA.n, RSA.fixed_length_of_bits_of_elements))
            fout.write(int_to_byte_arr(RSA.e, RSA.fixed_length_of_bits_of_elements))
            fout.write(int_to_byte_arr(RSA.d, RSA.fixed_length_of_bits_of_elements))
    
    # load the key elements (p, q, n, e, d)
    @staticmethod
    def load_ley():
        with open('key', 'r+b') as fin:
            byte_arr = b''
            single_byte = fin.read()
            
            while single_byte:
                byte_arr += single_byte
                single_byte = fin.read()
            
            fixed_length_of_bytes_of_elements = RSA.fixed_length_of_bits_of_elements // 8
            RSA.p = byte_arr_to_int(byte_arr[0:fixed_length_of_bytes_of_elements])
            RSA.q = byte_arr_to_int(byte_arr[fixed_length_of_bytes_of_elements:fixed_length_of_bytes_of_elements * 2])
            RSA.n = byte_arr_to_int(byte_arr[fixed_length_of_bytes_of_elements * 2:fixed_length_of_bytes_of_elements * 3])
            RSA.e = byte_arr_to_int(byte_arr[fixed_length_of_bytes_of_elements * 3:fixed_length_of_bytes_of_elements * 4])
            RSA.d = byte_arr_to_int(byte_arr[fixed_length_of_bytes_of_elements * 4:fixed_length_of_bytes_of_elements * 5])

    # do RSA encryption
    @staticmethod
    def do_encryption(plain_text):
        # load the key elements (p, q, n, e, d)
        RSA.load_ley()

        # convert the plain text into the list of numbers
        number_list = ascii_str_to_int_arr(plain_text)

        # the result string
        ret_str = ''
        
        # for each number in the list, do RSA ecnryption
        for x in number_list:
            y = NumberTheory.square_and_multiply(x, RSA.e, RSA.n)
            ret_str += hex(y)[2:] + ','
        # remove the last redundant ','
        ret_str = ret_str[0:len(ret_str) - 1]

        # write file
        with open('cipher_text', 'w') as fout:
            fout.write(ret_str)
        
        return ret_str
        
    # do RSA decryption
    @staticmethod
    def do_decryption(cipher_text):
        # load the key elements (p, q, n, e, d)
        RSA.load_ley()

        # convert the cipher text into the list of numbers
        number_list = [int('0x' + s, 16) for s in cipher_text.split(',')]

        # the result string
        ret_str = ''
        
        # for each number in the list, do RSA denryption using CRT for speed-up
        for y in number_list:
            x = NumberTheory.chinese_remainder_theorem(y, RSA.d, RSA.p, RSA.q)
            ret_str += chr(x)
        
        return ret_str

### ============================= ###

# main
if __name__ == '__main__':
    # the length of argv must be 2 or 3
    argc = len(sys.argv)
    if argc != 2 and argc != 3:
        print_exception('The number of argv must be 2 or 3',
                        '   Init command: \'RSA_B10615031.py init <number of bits>\'',
                        'Encrypt command: \'RSA_B10615031.py -e <plain  text>\'',
                        'Decrypt command: \'RSA_B10615031.py -d <cipher text>\'',
                        'Decrypt command: \'RSA_B10615031.py -b\'')
    else:
        # initialize RSA and generate the key
        if sys.argv[1] == 'init':
            RSA.init_rsa(int(sys.argv[2]))

        # do encryption
        elif sys.argv[1] == '-e':
            print(RSA.do_encryption(sys.argv[2]))
        
        # do decryption
        elif sys.argv[1] == '-d':
            print(RSA.do_decryption(sys.argv[2]))
        
        elif sys.argv[1] == '-b':
            with open('cipher_text', 'r') as fin:
                cipher_text = fin.read()
                print(RSA.do_decryption(cipher_text))

        # unknown commands
        else:
            print_exception('Unknown command \'{}\''.format(sys.argv[1]),
                            '   Init command: \'RSA_B10615031.py init <number of bits>\'',
                            'Encrypt command: \'RSA_B10615031.py -e <plain  text>\'',
                            'Decrypt command: \'RSA_B10615031.py -d <cipher text>\'',
                            'Decrypt command: \'RSA_B10615031.py -b\'')

