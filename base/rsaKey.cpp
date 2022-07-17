/*
 * rsaKey.cpp
 * Croix Gyurek
 *
 * Generates keys for RSA encryption.
 */

#include "mydefs.h"
#include "rsaKey.h"
#include <iostream>
#include <sstream>
#include <iomanip>

RSAKey::RSAKey() {
    // theseShouldNotBeUsed()
    N = BigNumber(10);
    E = BigNumber(3);
    D = BigNumber(3);
};

RSAKey::RSAKey(int keyBits, Randomizer generator) {
    generate(keyBits, generator);
};

RSAKey::RSAKey(BigNumber givenN, BigNumber givenE, BigNumber givenD) {
    N = givenN;
    E = givenE;
    D = givenD;
};
// For constructing only the public key.
// When User objects are read from database they only know N and E
// the D has to be obtained from the user's password
RSAKey::RSAKey(BigNumber givenN, BigNumber givenE) {
    N = givenN;
    E = givenE;
    D = BigNumber(0);
};

BigNumber RSAKey::getN() {
    return N;
};
BigNumber RSAKey::getE() {
    return E;
};
BigNumber RSAKey::getD() {
    return D;
};
void RSAKey::recordD(BigNumber& newD) {
    D = newD;
};
void RSAKey::generate(int keyBits, Randomizer generator) {
    std::cout << "Generating first prime..." << std::endl;
    // to generate a key of size N you multiply 2 primes of size N/2
    BigNumber p = BigNumber::generateLikelyPrime(keyBits/2 - 4, generator);
    std::cout << "Generating second prime..." << std::endl;
    BigNumber q = BigNumber::generateLikelyPrime(keyBits/2 + 4, generator);
    std::cout << "Generating decryption key..." << std::endl;

    // found the first part of our key
    this->N = p * q;
    // phi(N) = number of integers in 1..N-1 relatively prime to N
    // if N is a product of 2 primes p1,p2 then phi(N) = (p1-1)*(p2-1)
    BigNumber phiN = (p - BigNumber(1)) * (q - BigNumber(1));
    
    // preferred choices for encryption key (e)
    // usually 65537 suffices but I'm paranoid
    // altho you have a better chance of getting a composite p or q than failing even the first 2-3 of these
    // these are primes between 2^16 and 2^17 with at most 4 "1" bits
    int eChoices[] = {
        65537, 65539, 65543, 65557, 65609,
        65617, 65633, 65729, 65809, 65921,
        66569, 66593, 67073, 67589, 67601,
        69697, 69761, 70657, 81929, 81937,
        81953, 83969, 86017, 98321, 98369,
        98561, 114689
    };
    int numEs = 27;
    bool keepGoing = true;
    for (int i = 0; i < numEs && keepGoing; i++) {
        BigNumber candidate(eChoices[i]);
        // try to find k s.t. k*phiN + 1 is divisible by this candidate
        for (int k = 1; k < eChoices[i] && keepGoing; k++) {
            BigNumber kPhiPlus1 = BigNumber(k) * phiN + BigNumber(1);
            // check if kPhiPlus1 mod E is zero
            if ((kPhiPlus1 % candidate).isZero()) {
                // correct k found
                this->E = candidate;
                // then get the decryption key (mod N, of course)
                this->D = (kPhiPlus1 / candidate) % this->N;
                // no need to keep going
                keepGoing = false;
            }
        }
    }
    if (this->E.isZero()) {
        // PANIC!
        std::cerr << "[RSAKey::generate] Error: Something freaky happened!" << std::endl;
    }
};

// this is actual plain text we are passing in
std::string RSAKey::encrypt(std::string plainText) {
    int messageLength = plainText.length();
    if (messageLength > MESSAGE_CAP) {
        throw std::string("Error: Your message is too long.");
    }

    std::stringstream cipherStream;
    
    // loop over in 100-character increments
    int chunkSize = 100;
    for (int pos = 0; pos < messageLength; pos += chunkSize) {
        std::string chunk = plainText.substr(pos, chunkSize);
        std::stringstream ss;
        int chunkLength = chunk.length();
        for (int i = 0; i < chunkLength; i++) {
            int charCode = (int)chunk[i];
            ss << std::setfill('0') << std::setw(2) << std::hex;
            ss << charCode;
        }
        // randomness is so the same message encrypted twice does not give the same ciphertext
        Randomizer gen(chunk);
        // 32 hex is 128 bits. 100 characters is 800 bits. 928 fits nicely
        ss << gen.randomHex(32);
        // convert the hex we have accumulated into a BigNumber
        BigNumber M(ss.str());

        // The Moment of Truth!! (E,N come from this)
        BigNumber cipher = BigNumber::modExp(M, E, N);
        // comma is so we can separate them later
        cipherStream << cipher << ',';
    }
    return cipherStream.str();
};

std::string RSAKey::decrypt(std::string cipherText) {
    if (D.isZero()) {
        // Decryption failed: no key!
        return cipherText;
    }
    std::string message;
    // Put the cipher text into a stream so we can parse out the parts.
    std::stringstream cipherStream;
    cipherStream << cipherText;
    std::string cipherChunk;
    while (getline(cipherStream, cipherChunk, ',')) {
        // Check for non-empty.
        if (cipherChunk.length() > 0) {
            // Make a BigNumber out of it!
            BigNumber c(cipherChunk);
            // Decrypt.
            BigNumber decrypted = BigNumber::modExp(c, D, N);
            // Get the shift of 4 units to the right
            decrypted = decrypted.getShift(-4);
            // Convert that decrypted number back to hex
            std::string messageHex = decrypted.convertToHex();
            int hexLength = messageHex.length();
            std::stringstream plainStream;
            // Get all the hex turned into chars
            for (int i = 0; i < hexLength; i += 2) {
                std::stringstream converter;
                int charCode;
                converter << 1 << messageHex[i] << messageHex[i+1];
                converter >> std::hex >> charCode;
                // there might be zeros in this
                charCode -= 256;
                if (charCode != 0) {
                    plainStream << (char)charCode;
                }
            }
            // we have part of the message now
            message = message + plainStream.str();
        }
    }

    return message;
};


void RSAKey::test() {
    Randomizer r1("8932c#IU38WIO@ueshfuiwe");
    RSAKey rk1(1000, r1);
    // make sure N,E,D work
    BigNumber n1 = rk1.getN();
    BigNumber e1 = rk1.getE();
    BigNumber d1 = rk1.getD();
    
    std::cout << n1 << std::endl
              << e1 << std::endl
              << d1 << std::endl;
    
    std::cout << "Encrypting small number" << std::endl;
    BigNumber testM(42);
    BigNumber testC = BigNumber::modExp(testM, e1, n1);
    std::cout << "Encrypted: " << testC << std::endl;
    BigNumber testA = BigNumber::modExp(testC, d1, n1);
    std::cout << "Decrpyted: " << testA << std::endl;
    
    // Encryption and Decryption
    std::string message("Hello. This is a longer message I am sending. "
        "Its purpose is to test the encryption method when the key "
        "has to work on messages approaching the 500 character limit.\n"
        "As you can see, this is going to be rather long, because 500 "
        "is a LOT of characters. I am getting about 60 characters per "
        "line here. So this sentence should take me to 360 characters, "
        " so I need 140 more.\nOf course, I cannot go above the 500 limit, "
        " so I should be safe and stop... here.");
    std::cout << "Message length: " << message.length() << std::endl;
    std::string cipher = rk1.encrypt(message);
    std::cout << "Encrypted: " << cipher << std::endl;
    std::string plain = rk1.decrypt(cipher);
    std::cout << "Decrypted: " << plain << std::endl;
    std::cout << "Should be: " << message << std::endl;
};


