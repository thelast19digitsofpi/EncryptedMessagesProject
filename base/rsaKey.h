/*
 * rsaKey.h
 * Croix Gyurek
 *
 * Generates RSA key pairs of a certain size.
 * THE NULL CONSTRUCTOR GENERATES TRIVIAL KEYS. DO NOT USE IT.
 */

#ifndef RSAKEY_H_EXISTS
#define RSAKEY_H_EXISTS

#include <string>
#include "bigNumber.h"
#include "randomizer.h"

// Note: The class object itself should be kept hidden because it can expose the D key!
class RSAKey {
    protected:
        BigNumber N;
        BigNumber E;
        BigNumber D;
    public:
        RSAKey();
        RSAKey(int, Randomizer);
        RSAKey(BigNumber, BigNumber, BigNumber);
        // for partial (public-only) keys
        RSAKey(BigNumber, BigNumber);
        
        BigNumber getN();
        BigNumber getE();
        BigNumber getD();
        // this can also be used to sanitize the key
        void recordD(BigNumber&);
        void generate(int, Randomizer);
        std::string encrypt(std::string);
        std::string decrypt(std::string);

        static void test();
};


#endif

