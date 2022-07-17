/*
 * sha256.cpp
 * Croix Gyurek
 *
 * Secure Hash Algorithm 2, 256-bit
 */

#include "sha.h"
#include <iomanip>
#include <cstdio>

shaInt rightRotate(shaInt num, int places) {
    // e.g. rotating 11001001 right by 3 returns (001)(11001) or 00111001
    // except it would not because shaInt is 32 bits not 8
    // note: x & ((1 << n) - 1) means "get the last n bits"
    shaInt left = num >> places;
    shaInt right = num & ((1 << places) - 1);
    return (right << (32 - places)) | left;
};

#include <openssl/sha.h>
// I got this implementation from stack overflow
// question 2262386
// the OpenSSL docs are impossible
std::string sha256(std::string thingToHash) {
    SHA256_CTX sha256;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    // I never understand why they always do it this way
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, thingToHash.c_str(), thingToHash.size());
    SHA256_Final(hash, &sha256); // and exactly why did these params switch places?
    
    // Oh neat. I've done this step before.
    // I guess I'm in good company.
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setfill('0') << std::setw(2) << (int)hash[i];
    }
    return ss.str();
};

std::string sha256_broken(shaByte* byteArray, int msgBytes) {
    int i;
    // these are apparently called "hash values"
    shaInt hv[8] = {0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
                    0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U};
    // these are apparently called "round constants"
    shaInt k[64] = {
        0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
        0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
        0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
        0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
        0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
        0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
        0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
        0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
        0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U, 
        0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
        0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U, 
        0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
        0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U, 
        0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
        0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U, 
        0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U
    };
    
    // pad the byteArray
    // we want K so that 8*L + 8*K + 64 is a multiple of 512
    // and then we add 8 further bytes for the message length
    // i.e. L + K + 8 is a multiple of 64
    // i.e. K + 8 = (-L) % 64 but must be > 0 so we add 64
    int numExtraBytes = ((-msgBytes) % 64) + 64;
    int totalBytes = msgBytes + numExtraBytes;
    shaByte* paddedMessage = new shaByte[totalBytes];
    // copy over the bytes
    for (i = 0; i < msgBytes; i++) {
        paddedMessage[i] = byteArray[i];
    }
    // add extra padding
    paddedMessage[msgBytes] = (shaByte)0x80u; // a single 1 followed by 7 zeros
    for (i = msgBytes + 1; i < totalBytes - 8; i++) {
        paddedMessage[i] = (shaByte)0u;
    }
    // finally insert the length of the message
    // here i is not the array index
    int numBits = msgBytes * 8;
    for (i = 0; i < 8; i++) {
        shaByte lengthByte = (shaByte)(numBits % 256);
        paddedMessage[totalBytes - 1 - i] = lengthByte;
        numBits = numBits >> 8; // clear those bits
    }
    
    /*
    printf("%d: ", totalBytes);
    for (i = 0; i < totalBytes; i++) {
        printf("%02x ", paddedMessage[i]);
    }
    printf("\n");
*/

    // note: "numBits" is now destroyed
    // proceed to the hashing!
    // process message in 64-byte chunks (512 bits)
    for (int chunk = 0; chunk < totalBytes; chunk += 64) {
        shaInt w[64]; // called the message schedule array (MSA)
        // copy the chunk into the first 16 slots on the MSA... somehow
        for (i = 0; i < 16; i++) {
            // put the corresponding 4 bytes in
            shaInt fourBytes = 0u;
            for (int j = 0; j < 4; j++) {
                fourBytes = fourBytes << 8u;
                fourBytes += (shaInt)paddedMessage[chunk + 4*i + j];
            }
            w[i] = fourBytes;
        }
        // now fill in the other 48 slots
        for (i = 16; i < 64; i++) {
            // ^ is bitwise XOR, not exponentiation
            shaInt s0 = rightRotate(w[i - 15], 7) ^ rightRotate(w[i - 15], 18) ^ (w[i - 15] >> 3);
            shaInt s1 = rightRotate(w[i-2], 17) ^ rightRotate(w[i-2], 19) ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }
        // these are the "working variables" I guess
        shaInt a = hv[0],
               b = hv[1],
               c = hv[2],
               d = hv[3],
               e = hv[4],
               f = hv[5],
               g = hv[6],
               h = hv[7];
        // turn on the blender! (and remember to put on the lid)
        for (i = 0; i < 63; i++) {
            shaInt S1 = rightRotate(e, 6) ^ rightRotate(e, 11) ^ rightRotate(e, 25);
            shaInt ch = (e & f) ^ ((~e) & g);
            shaInt temp1 = h + S1 + ch + k[i] + w[i];
            shaInt S0 = rightRotate(a, 2) ^ rightRotate(a, 13) ^ rightRotate(a, 22);
            shaInt maj = (a & b) ^ (a & c) ^ (b & c); // majority bit from a,b,c I assume?
            shaInt temp2 = S0 + maj;

            // give it a stir
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }
        hv[0] += a;
        hv[1] += b;
        hv[2] += c;
        hv[3] += d;
        hv[4] += e;
        hv[5] += f;
        hv[6] += g;
        hv[7] += h;
    }
    // The final hash value I have to return as a string... ugh
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0');
    for (int i = 0; i < 8; i++) {
        ss << hv[i];
    }
    // that wasn't too bad...
    std::string finalHash;
    getline(ss, finalHash);

    delete[] paddedMessage;
    return finalHash;
};

// it is usually easier to hash a string than a byte array
std::string sha256_broken(std::string thingToHash) {
    int numBytes = thingToHash.length();
    shaByte* buffer = new shaByte[numBytes];
    for (int i = 0; i < numBytes; i++) {
        buffer[i] = (shaByte)thingToHash[i];
    }

    std::string hash = sha256_broken(buffer, numBytes);
    delete[] buffer; // always remember to delete!
    return hash;
};

#include <iostream>
void testSHA() {
    std::cout << "Right rotate test: " << std::hex << rightRotate(0x12345678u, 4) << std::dec << std::endl;
    std::cout << "Empty hash: " << sha256("") << std::endl;
    std::cout << "Hash of \"abc\": " << sha256("abc") << std::endl;

    std::cout << "Really long hash: " << sha256(
            "18736278643746137264871634781623478634"
            "12346387628736418723648723648732648723"
            "18234723498378327584757346574329548754"
            "aaaaaaaaaaaaaaaaaaaaaaaaaaaacaaaaaaaaa"
            "ababababaj{{}{{{{{{{{^$***mmmmmmmmmmnn"
    ) << std::endl;
};


