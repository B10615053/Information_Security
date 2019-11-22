#!/usr/bin/env python
# coding: utf-8

# In[54]:


import sys
import os
from PIL import Image
from Crypto.Util.Padding import pad, unpad
from Crypto.Cipher import AES

### ============================= ###

# for printing the formatted exception
def print_exception(main_message, *sub_messages):
    print('\n\t>>> EXCEPTION: {}'.format(main_message))
    for msg in sub_messages:
        print('\t\t\t', msg)
    print()

### ============================= ###

# make sure that the output image path has a non-".ppm" extension, e.g. .jpg, .png
def readjust_output_image_path(original_output_image_path, input_image_extension_with_dot='.png'):
    # if users didn't type any characters for the output image, set a default name
    if len(original_output_image_path) == 0:
        return 'it_is_the_default_output_image_name_since_you_didnt_type_any_characters____why_you_didnt_type_it.png'
    
    # the extension typed by users is .ppm, replace it w/ the extension of the input image
    elif original_output_image_path.endswith('.ppm'):
        return original_output_image_path[0:len(original_output_image_path) - 4] + input_image_extension_with_dot
    
    # already has a non-ppm extension
    elif '.' in original_output_image_path:
        return original_output_image_path
    
    # no extension comprised, add the one from the input image
    else:
        return original_output_image_path + input_image_extension_with_dot

### ============================= ###

# before doing the cipher, make sure get the correct and valid arguments
def preprocess(argv):
    # get the path of input image
    input_image_path = argv[1]
    
    # get the path of output image
    output_image_path = readjust_output_image_path(argv[2],
                           '.png' if '.' not in input_image_path else input_image_path[input_image_path.rfind('.'):])
    
    print('Input image path: \'' + input_image_path + '\'')
    print('Output image path: \'' + output_image_path + '\'')
    
    # get the key for the cipher
    key = argv[3]
    
    # get the block cipher mode
    block_cipher_mode = argv[4]
    
    # if there's an explicit IV typed by users
    if len(argv) == 6:
        initialization_vector = argv[5]
    # else, set IV to the hex of all zeros by default
    else:
        initialization_vector = 16 * '\x00'
    
    # try to open as an image
    try:
        # open the image expected with RGB color mode
        raw_image = Image.open(input_image_path)
        
        # if it's not, convert the color mode into RGB
        if raw_image.mode != 'RGB':
            raw_image = raw_image.convert('RGB')
    
    # exception: no file found for the input image
    except FileNotFoundError:
        print_exception('No such file found with your input-image-path',
                        'The path you typed is \'' + input_image_path + '\'',
                        'Please make sure there\'s no typo, or the extension is correct (like jpg, png, ...)',
                        'Please try again')
        
    # exception: the input image is NOT an image or PIL library doesn't support it
    except (OSError, PermissionError):
        print_exception('Your input-image-path is NOT an image file or PIL library doesn\'t support it',
                        'The path you typed is \'' + input_image_path + '\'',
                        'Please try again')
    
    # exception: null input or other unknown errors
    except:
        print_exception('No any input or the unknown error raised')
    
    else:
        # set the image path for ppm format
        ppm_image_path = output_image_path + '.ppm'

        # generate the ppm file
        raw_image.save(ppm_image_path)

        # close the raw image
        raw_image.close()

        # open the image as the ppm format
        with Image.open(ppm_image_path) as ppm_image:
            # get the width and height of the image
            width = ppm_image.size[0]
            height = ppm_image.size[1]

            # get whole pixels of the image and build the plain text as a bytearray
            plain_text = b''
            for y in range(height):
                for x in range(width):
                    # a pixel has 3 channels (R, G, and B) in the color mode of RGB
                    for channel in ppm_image.getpixel((x, y)):
                        # get a byte of the channel of the pixel
                        a_byte = channel.to_bytes(1, 'little')

                        # build the plain text which is actually a bytearray
                        plain_text += a_byte
        
            # do the decryt cipher algorithm
            cipher_text = do_cipher(plain_text, key, block_cipher_mode, initialization_vector)
            
            # set the decrypted output ppm image
            cipher_idx = 0
            for y in range(height):
                for x in range(width):
                    # a unit of 3 has R, G, and B orderly
                    red = cipher_text[cipher_idx]
                    green = cipher_text[cipher_idx + 1]
                    blue = cipher_text[cipher_idx + 2]
                    
                    # the index of cipher text adds by 3 every iteration
                    cipher_idx += 3
                    
                    # put the pixel into the ppm image
                    ppm_image.putpixel((x, y), (red, green, blue))
            
            # save to the designated output image path as a NON-ppm type of image
            ppm_image.save(output_image_path)
            
            # show the image
            ppm_image.show()
            
            print('\t\tDecryption finished')

### ============================= ###

# do the cipher algorithm
def do_cipher(cipher_text, key, mode, iv):
    # get the length of the cipher text
    text_length = len(cipher_text)
    
    # build an AES cryptor with ECB mode
    ecb_cryptor = AES.new(key.encode('utf8'), AES.MODE_ECB)
    
    # create a iterative-IV for every non-ECB block cipher, initially set to IV
    iter_iv = iv.encode('utf8')
    
    plain_text = b''
    idx = 0
    while idx < text_length:
        # get a block
        block = cipher_text[idx:idx + 16]
        
        # if mode is SOC
        if mode == 'SOC':
            # encrypt the iterative-IV
            encrypted_iter_iv = ecb_cryptor.encrypt(iter_iv)
            
            # get the rightmost bit of the encrypted iterative-IV
            rightmost_bit_of_encrypted_iter_iv = (encrypted_iter_iv[15] & 1)
            
            # if the rightmost bit is 0, shift left by a bit circularly
            if rightmost_bit_of_encrypted_iter_iv == 0:
                iter_iv = shift_left_a_bit_circularly(iter_iv)
            # if the rightmost bit is 1, plus by one
            else:
                iter_iv = plus_one(iter_iv)
            
            # the block plain text is just the result of exclusive-or w/ encrypted iterative-IV and block cipher text
            plain_text += xor(block, encrypted_iter_iv)
        
        # if mode is ECB or CBC
        else:
            # pad the block if it needs
            if len(block) < 16:
                block = pad(block, 16)
            
            # do block cipher
            decrypted = ecb_cryptor.decrypt(block)
            
            # CBC mode: do exclusive-or with iv or the previous blocked cipher-text
            if mode == 'CBC':
                decrypted = xor(decrypted, iter_iv)
                iter_iv = block
            
            plain_text += decrypted
        
        idx += 16
    
    return plain_text

### ============================= ###

# pad the hex string to make sure its length is multiple of 16 bytes
# use the method of PKCS
def pads(hex_string):
    length = len(hex_string)
    
    # need to be padded
    if length % 16 != 0:
        num_of_padding = 16 - (length % 16)
        num_of_padding_in_hex_string = hex(num_of_padding).encode('utf8')
        
        ret = hex_string + (num_of_padding_in_hex_string * num_of_padding)
        return ret
    
    # no need, return directly
    else:
        return hex_string
    
### ============================= ###

# shift a bytearray left by a bit circularly
def shift_left_a_bit_circularly(byte_arr):
    modulus = 2 ** 8
    and_mask = 254
    
    leftmost_byte = byte_arr[0]
    leftmost_bit = ((leftmost_byte >> 7) & 1)
    
    len_of_byte_arr = len(byte_arr)
    
    ret = b''
    for byte_idx, single_byte in enumerate(byte_arr):
        new_byte = (((single_byte << 1) % modulus) & and_mask)
        if byte_idx == len_of_byte_arr - 1:
            new_byte = (new_byte | leftmost_bit)
        else:
            new_byte = (new_byte | ((byte_arr[byte_idx + 1] >> 7) & 1))
        ret += new_byte.to_bytes(1, 'little')
    
    return ret

### ============================= ###

# return the result of a bytearray + 1
def plus_one(byte_arr):
    modulus = 2 ** 8
    
    rev_ret = b''
    carry = 1
    for single_byte in reversed(byte_arr):
        the_sum = single_byte + carry
        
        remainder = the_sum % modulus
        quotient = the_sum // modulus
        
        rev_ret += remainder.to_bytes(1, 'little')
        carry = quotient
    
    ret = b''
    for single_byte in reversed(rev_ret):
        ret += single_byte.to_bytes(1, 'little')
    
    return ret

### ============================= ###

# do exclusive-or with two bytearrays
def xor(lhs, rhs):
    ret = b''
    for idx, _ in enumerate(lhs):
        ret += (lhs[idx] ^ rhs[idx]).to_bytes(1, 'little')
    return ret

### ============================= ###

# main
if __name__ == '__main__':
    # the length of argv must be 5, or 6 (includes IV or not)
    argc = len(sys.argv)
    if argc != 5 and argc != 6:
        print_exception('The number of argv must be 5 or 6 (includes IV or not)',
                        'Valid command format: \'Decrypt.py <input_image> <output_image> <key> <mode> [<IV>]\'',
                        'Note that the images could be absolute paths or relative paths',
                        'Note that the key must be the length of 16 bytes',
                        'If the file path or key comprises any white space, you must quote it with a pair of \"\"',
                        'Note that the mode must only be \'ECB\' or \'CBC\'')
        
        # for testing
#         t = ('', 'encrypted.png', 'za_warudo.png', 'T78hgiQs+-0 8i[p', 'SOC', 'q*s[ty=d-029wjai')
#         preprocess(t)
    else:
        preprocess(sys.argv)


# In[ ]:




