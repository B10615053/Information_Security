#!/usr/bin/env python
# coding: utf-8

import sys
import os
from Crypto.Cipher import AES
from random import randint
from random import randrange
from hashlib import sha1

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

# convert a binary string into an integer
def bin_str_to_int(bin_str):
    return int('0b' + bin_str, 2)

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

# the DSA class
class DSA():
    p_bits = 1024
    q_bits = 160

    @staticmethod
    def generate_and_save_key():
        print('\tGenerating the key, please wait...')

        # step 1. generate a prime q of 160 bits
        q = NumberTheory.generate_prime_randomly(DSA.q_bits)

        # step 2. generate a prime p of 1024 bits where p = k * q + 1
        min_k = 2 ** (DSA.p_bits - DSA.q_bits - 1)
        max_k = min_k * 2
        k = randrange(min_k, max_k)
        p = k * q + 1

        # step 1 + 2, if the first p is NOT prime, re-choose p and q until it is
        min_p = 2 ** (DSA.p_bits - 1)
        while p < min_p or (not NumberTheory.check_prime(p, 1)):
            q = NumberTheory.generate_prime_randomly(DSA.q_bits)
            k = randrange(min_k, max_k)
            p = k * q + 1
            if p < min_p:
                min_k = k + 1
        
        # for debugging
        # print('p =', p, ', len =', len(int_to_bin_str(p)))
        # print('q =', q, ', len =', len(int_to_bin_str(q)))
        
        # step 3. find an integer alpha that alpha = (h^((p - 1) / q)) mod p
        h = randint(10, 100)
        alpha = NumberTheory.square_and_multiply(h, (p - 1) // q, p)

        # step 4. choose a random integer d w/ 0 < d < q
        d = randint(1, q - 1)

        # step 5. compute beta = (alpha^d) mod p
        beta = NumberTheory.square_and_multiply(alpha, d, p)

        # step 6. save the key elements
        # step 6-1. save the public key elements
        with open('key_pub', 'w') as fout:
            fout.write(str(p) + ",")
            fout.write(str(q) + ",")
            fout.write(str(alpha) + ",")
            fout.write(str(beta))
        # step 6-2. save the private key element
        with open('key_pr', 'w') as fout:
            fout.write(str(d))
        
        print('\tKey generation finished')

    @staticmethod
    def sign(message):
        # pre-step. get the public and private key (p, q, alpha, beta, d)
        p, q, alpha, beta, d = DSA.load_key()

        # step 1. choose an integer as random ephemeral key (k_E)
        k_E = randrange(1, q)
        
        # step 2. compute r = ((alpha ^ k_E) mod p) mod q
        r = NumberTheory.square_and_multiply(alpha, k_E, p) % q

        # step 3. compute s = ((sha1(x) + (d * r)) * inverse(k_E)) mod q, where x is the message
        # step 3-1. hash the message through SHA-1
        digest = DSA.do_hashing(message)
        # step 3-2. calculate the inverse of the ephemeral key (k_E) w/ the modulus of q
        # use the theory that a^(p-2) is the inverse of a w/ the modulus of p
        inv_k_E = NumberTheory.square_and_multiply(k_E, q - 2, q)
        # step 3-3. compute s
        s = ((digest + (d * r)) * inv_k_E) % q
        
        # for debugging
        # print('r =', r)
        # print('s =', s)
        
        # step 4. save the signature (r and s)
        with open('signature', 'w') as fout:
            fout.write(str(r) + ",")
            fout.write(str(s))
        
        print('\tSignature generated')
    
    @staticmethod
    def verify(message):
        # pre-step. get the public key (p, q, alpha, beta)
        p, q, alpha, beta = DSA.load_key(should_get_private=False)
        # pre-step. get the signature (r and s)
        r, s = DSA.load_signature()

        # step 1. compute auxiliary value w = inverse(s) mod q
        inv_s = NumberTheory.square_and_multiply(s, q - 2, q)
        w = inv_s % q
        
        # step 2. compute auxiliary value u_1 = (w * sha1(x)) mod q, where x is the message
        # step 2-1. hash the message through SHA-1
        digest = DSA.do_hashing(message)
        # step 2-2. compute u1
        u1 = (w * digest) % q

        # step 3. compute auxiliary value u2 = (w * r) mod q
        u2 = (w * r) % q

        # step 4. compute v = ((alpha^u1 * beta^u2) mod p) mod q
        alpha_de_u1_cifang = NumberTheory.square_and_multiply(alpha, u1, p)
        beta_de_u2_cifang = NumberTheory.square_and_multiply(beta, u2, p)
        v = ((alpha_de_u1_cifang * beta_de_u2_cifang) % p) % q

        # step 5. validate the signature
        if v == (r % q):
            return 'valid'
        else:
            return 'invalid'
    
    @staticmethod
    def do_hashing(x):
        return int(sha1(x.encode('utf-8')).hexdigest(), 16)

    @staticmethod
    def load_key(should_get_public=True, should_get_private=True):
        ret = []

        # get the public key (p, q, alpha, beta)
        if should_get_public:
            with open('key_pub', 'r') as fin:
                line = fin.read()
                ret.extend([int(ele) for ele in line.split(',')])

        # get the private key (d)
        if should_get_private:
            with open('key_pr', 'r') as fin:
                ret.append(int(fin.read()))

        return ret

    @staticmethod
    def load_signature():
        with open('signature', 'r') as fin:
            line = fin.read()
            return [int(ele) for ele in line.split(',')]
    
### ============================= ###

# main
if __name__ == '__main__':
    # the length of argv must be 3
    argc = len(sys.argv)
    if argc != 3:
        print_exception('The number of argv must be 3',
                        '  Key-gen command: \'DSA.py -keygen 160\'',
                        '  Signing command: \'DSA.py -sign <message>\'',
                        'Verifying command: \'DSA.py -verify <message>\'')
    else:
        # generate the key
        if sys.argv[1] == '-keygen':
            if not str.isdigit(sys.argv[2]) or int(sys.argv[2]) != 160:
                print_exception('The length of q could ONLY be 160 bits')
            else:
                DSA.generate_and_save_key()

        # do signing
        elif sys.argv[1] == '-sign':
            DSA.sign(sys.argv[2])
            pass
        
        # do verifying
        elif sys.argv[1] == '-verify':
            verification_result = DSA.verify(sys.argv[2])
            print('\tVerification result:', verification_result)
            if verification_result == 'invalid':
                print('\tThe message has been modified!')

        # unknown commands
        else:
            print_exception('Unknown command \'{}\''.format(sys.argv[1]),
                            '  Key-gen command: \'DSA.py -keygen 160\'',
                            '  Signing command: \'DSA.py -sign <message>\'',
                            'Verifying command: \'DSA.py -verify <message>\'')

