import sys
import os
from PIL import Image
from Crypto.Util.Padding import pad, unpad
from Crypto.Cipher import AES

### ============================= ###

# Make sure the output image has extention of .jpg or .png
def checkOutputExtention(outputImagePath, inputImageExtention = '.png'):
    # User didn't set output file name, so set file name as default NO_FILE_NAME.png
    if len(outputImagePath) == 0:
        return 'NO_FILE_NAME.png'
    # If the output image has extention of .ppm, set output extention same as the input image
    elif outputImagePath.endswith('.ppm'):
        return outputImagePath[0 : len(outputImagePath) - 4] + inputImageExtention
    # Output image already had extention
    elif '.' in outputImagePath:
        return outputImagePath
    # No extention in output image path
    else:
        return outputImagePath + inputImageExtention

### ============================= ###

# Exception printer
def printException(mainMessage, *subMessages):
    print('\n\t>>> EXCEPTION: {}'.format(mainMessage))
    for message in subMessages:
        print('\t\t\t', message)

### ============================= ###

# Pad the hex string to make sure its length is multiple of 16 bytes
# use the method of PKCS
def padding (hexString):
    length = len(hexString)
    # Check whether need to padding
    if length % 16 != 0:
        insufficient = 16 - (length % 16)
        complementPart = hex(insufficient).encode('utf8')
        answer = hexString + (complementPart * insufficient)
        return answer
    # No need to padding
    else:
        return hexString

### ============================= ###

def xor (left, right):
    answer = b''
    for index, _ in enumerate(left):
        answer += (left[index] ^ right[index]).to_bytes(1, 'little')
    return answer

### ============================= ###

def leftShift (byteArray):
    modulus = 2 ** 8
    mask = 254
    leftmostByte = byteArray[0]
    leftmostBit = ((leftmostByte >> 7) & 1)
    length = len(byteArray)
    answer = b''
    for index, byte in enumerate(byteArray):
        shiftedByte = (((byte << 1) % modulus) & mask)
        if index == length - 1:
            shiftedByte = (shiftedByte | leftmostBit)
        else:
            shiftedByte = (shiftedByte | ((byteArray[index + 1] >> 7) & 1))
        answer += shiftedByte.to_bytes(1, 'little')
    return answer

### ============================= ###

def plusOne (byteArray):
    modulus = 2 ** 8
    reverseAnswer = b''
    carry = 1
    for byte in reversed(byteArray):
        sum = byte + carry
        remainder = sum % modulus
        quotient = sum // modulus
        reverseAnswer += remainder.to_bytes(1, 'little')
        carry = quotient
    answer = b''
    for byte in reversed(reverseAnswer):
        answer += byte.to_bytes(1, 'little')
    return answer

### ============================= ###

# Do the encrypt algorithm
def encrypter(plainText, key, mode, iv):
    cipherText = b''
    # Get the length of padded text
    length = len(plainText)
    # Create an AES cryptor with ECB mode
    cryptor = AES.new(key.encode('utf8'), AES.MODE_ECB)
    # Create a iterative initialization vector
    iterativeIV = iv.encode('utf8')
    # Block index
    blockIndex = 0
    while blockIndex < length:
        # Mode is ECB or CBC
        if mode == 'ECB' or mode == 'CBC':
            # Get a block
            block = plainText[blockIndex :blockIndex + 16]
            # Padding the block if it length is less than 16 bytes
            if len(block) < 16:
                block = pad(block, 16)
            # ECB mode: only use block's plain text and key to cipher, and block is encrypted separately.
            if mode == 'ECB':
                block = cryptor.encrypt(block)
                cipherText += block
            # CBC mode: first XOR block's plain text and last block's cipher text(except first block use IV), then use key and answer to cipher.
            #           The encryption of all blocks are chained together.
            elif mode == 'CBC':
                block = xor(block, iterativeIV)
                block = cryptor.encrypt(block)
                iterativeIV = block
                cipherText += block
        # SOC mode: first XOR block's plain text and last block's cipher text(except first block use IV), then use key and answer to cipher.
            # Get right most byte in the block
        elif mode == 'SOC':
            # Cypher the block
            block = cryptor.encrypt(iterativeIV)
            # Get byte in the block
            rightMostByte = block[15] & 1
            # Check right most bit
            if rightMostByte == 0:
                # Shift left iterative initialization vector
                iterativeIV = leftShift(iterativeIV)
            else:
                # Plus interative initialization vector for 1
                iterativeIV = plusOne(iterativeIV)
            block = xor(plainText[blockIndex :blockIndex + 16], block)
            cipherText += block
        blockIndex += 16

    return cipherText

### ============================= ###

def preprocess(argv):
    # Get the path of original image
    originalImagePath = argv[1]
    # Get the path of encrypted image
    encryptedImagePath = checkOutputExtention(argv[2],
                                        '.png' if '.' not in originalImagePath else originalImagePath[originalImagePath.rfind('.'):])
    # Show original image path and encrypted image path
    print('Original Image Path: \'' + originalImagePath + '\'')
    print('Encrypted Image Path: \'' + encryptedImagePath + '\'')
    # Get key
    key = argv[3]
    # Get block cipher mode
    blockCipherMode = argv[4]
    # Check whether user give initialization vector
    if len(argv) == 6:
        initializationVector = argv[5]
    else:
        initializationVector = 15 * '\x00' + '\x01'
    # Try to open as an image
    try:
        # Open the original image with RGB color mode
        rawImage = Image.open(originalImagePath)
        # Check whether the raw image has mode of RGB
        if rawImage.mode != 'RGB':
            rawImage = rawImage.convert('RGB')
    # Exception: no file found for the original image
    except FileNotFoundError:
        printException ('No such file found with your original image path',
                        'The path you typed is \'' + originalImagePath + '\'',
                        'Please make sure there\'s no typo, or the extension is correct (like jpg, png, ...)',
                        'Please try again')
    # Exception: the original image is NOT an image or Pillow Library doesn't support it
    except (OSError, PermissionError):
        printException ('Your original image path is NOT an image file or Pillow Library doesn\'t support it',
                        'The path you typed is \'' + originalImagePath + '\'',
                        'Please try again')
    # Exception: null input or other unknown errors
    except:
        printException('No input or unknown error raised')
    # Normal
    else:
        # Set the image path for .ppm format
        ppmImagePath = encryptedImagePath + '.ppm'
        # Create the .ppm file
        rawImage.save(ppmImagePath)
        # Close the raw image
        rawImage.close()
        # Open the .ppm format image
        with Image.open(ppmImagePath) as ppmImage:
            # Get the width and height of the .ppm image
            width = ppmImage.size[0]
            height = ppmImage.size[1]
            # Save plain text as bytes
            plainText = b''
            # Get .ppm image's pixels and transfer them as bytes
            for h in range(height):
                for w in range(width):
                    # Per pixel has 3 channels(R, G, B)
                    for channel in ppmImage.getpixel((w, h)):
                        # Get byte of the pixel
                        byte = channel.to_bytes(1, 'little')
                        # Dump the byte into plain text
                        plainText += byte
            # Encrypt algorithm
            cipherText = encrypter(plainText, key, blockCipherMode, initializationVector)
            # Set the encrypted text to ppm image
            pixelIndex = 0
            for h in range(height):
                for w in range(width):
                    # Seperate 3 channels(R, G, B)
                    red = cipherText[pixelIndex]
                    green = cipherText[pixelIndex + 1]
                    blue = cipherText[pixelIndex + 2]
                    # Jump to next pixel
                    pixelIndex += 3
                    # Set pixel into ppm image
                    ppmImage.putpixel((w, h), (red, green, blue))
            # Transfer ppm image to encrypted image extention type
            ppmImage.save(encryptedImagePath)
            # Show the image
            ppmImage.show()
            print('\t\tEncryption finished')
    
### ============================= ###

# main
if __name__ == '__main__':
    argc = len(sys.argv)
    # The length of argv must be 5 or 6(Include initialization vector)
    if argc != 5 and argc != 6:
        printException('The number of argv must be 5 or 6 (includes IV or not)',
                        'Valid command format: \'Decrypt.py <input_image> <output_image> <key> <mode> [<IV>]\'',
                        'Note that the images could be absolute paths or relative paths',
                        'Note that the key must be the length of 16 bytes',
                        'If the file path or key comprises any white space, you must quote it with a pair of \"\"',
                        'Note that the mode must only be \'ECB\' or \'CBC\' or \'SOC\'')
    else:
        preprocess(sys.argv)
    
    