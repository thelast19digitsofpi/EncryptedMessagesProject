/*
 * bigNumber.h
 * Croix Gyurek
 *
 * Numbers of unlimited capacity
 */

#ifndef BIGNUMBER_H_EXISTS
#define BIGNUMBER_H_EXISTS

#include <cstdint>
#include <string>
#include <iostream>
#include <sstream>

#include "randomizer.h"

// numbers are d[0] + 256*d[1] + 256^2*d[2] + ...
#define BASE 4294967296ull
#define BASE_BITS 32
typedef uint32_t bndigit;
typedef uint64_t int64;

// The base of this number system is 2^16 (not 10)
// a bit like hexadecimal but compressed
class BigNumber {
    protected:
        // Unsigned char is 16 bits
        bndigit* digits;
        int numDigits;
    public:
        BigNumber(); // null constructor => 0
        BigNumber(bndigit*, int);
        // to make a small BigNumber i.e. from a single "digit"
        BigNumber(int64);
        BigNumber(std::string);
        BigNumber(const BigNumber&);
        BigNumber(BigNumber&);
        BigNumber(BigNumber&&);
        ~BigNumber();
        BigNumber& operator=(const BigNumber&);
        BigNumber& operator=(BigNumber&&) noexcept;
        
        // this function is dangerous
        // so you have to confirm you know what you are doing
        // by passing 0xbe5afe ("be safe") as parameter 3
        void directlyAssignDigits(bndigit*, int, int);

        int getNumDigits() const;
        bndigit getDigit(int) const;
        bool isZero() const;
        // useful if somehow the array has lots of leading 0s
        // deflate() actually copies memory
        // simpleDeflate() just changes the length parameter
        void deflate();
        void simpleDeflate();
        // gets the leading digit; very helpful
        bndigit getLeadingDigit() const;
        int getLeadingDigitPosition() const;

        BigNumber getShift(int) const;
        BigNumber getLowestNDigits(int) const;
        BigNumber& operator+=(const BigNumber&);
        BigNumber& operator-=(const BigNumber&);
        BigNumber& operator*=(const BigNumber&);
        BigNumber& operator/=(const BigNumber&);

        static BigNumber modExp(const BigNumber&, const BigNumber&, const BigNumber&);
        static BigNumber gcd(BigNumber&, BigNumber&);

        bool couldBePrime();
        bool fermatPrimalityTest();
        bool millerRabinTest();
        bool isProbablyPrime();
        static BigNumber generateLikelyPrime(int, Randomizer&);

        std::string convertToHex() const;
        friend std::ostream& operator<<(std::ostream&, const BigNumber&);

        void dumpDigits() const;
        static void test();
};

BigNumber operator+(BigNumber const&, BigNumber const&);
BigNumber operator-(BigNumber const&, BigNumber const&);
BigNumber operator*(BigNumber const&, BigNumber const&);
BigNumber operator/(BigNumber const&, BigNumber const&);
BigNumber operator%(BigNumber const&, BigNumber const&);

bool operator<(BigNumber const&, BigNumber const&);
bool operator>(BigNumber const&, BigNumber const&);
bool operator<=(BigNumber const&, BigNumber const&);
bool operator>=(BigNumber const&, BigNumber const&);
bool operator==(BigNumber const&, BigNumber const&);
bool operator!=(BigNumber const&, BigNumber const&);

#endif
