/*
 * bigNumber.cpp
 * Croix Gyurek
 *
 * I need numbers much larger than 18,446,744,073,709,551,615
 * to have a decent RSA encryption!
 */


#include "bigNumber.h"
#include <iostream>
#include <cstring>

// for testing
#include <cstdlib>
#include <chrono>

BigNumber::BigNumber() {
    digits = new bndigit[1];
    digits[0] = (bndigit)0;
    numDigits = 1;
};

BigNumber::BigNumber(bndigit* givenDigits, int arraySize) {
    digits = new bndigit[arraySize];
    // Doing this to try to cut back on copying time
    memcpy(digits, givenDigits, arraySize * sizeof(bndigit));
    numDigits = arraySize;
};

BigNumber::BigNumber(int64 smallNumber) {
    if (smallNumber < 0) {
        throw "Negative numbers not allowed in BigNumber";
    }
    
    // We do not know how many digits there will be
    bndigit buffer[4]; // 4 just in case "int" is 64 bits for some weird reason
    numDigits = 0; // use the class's variable; more efficient
    int64 valueLeft = smallNumber;
    do {
        // grab last 32 bits
        bndigit digit = (bndigit)(valueLeft & (BASE - 1ull));
        buffer[numDigits] = digit;
        numDigits++;
        // and knock down 32 bits
        valueLeft = valueLeft >> BASE_BITS;
    } while (valueLeft > 0);
    
    // now that we know how many digits, copy them into the correct place
    digits = new bndigit[numDigits];
    for (int i = 0; i < numDigits; i++) {
        digits[i] = buffer[i];
    }
    // done
};

void BigNumber::directlyAssignDigits(bndigit* newDigits, int newNumDigits, int beSafe) {
    if (beSafe != 0xbe5afe) {
        throw std::string("You need to prove you know what you are doing.");
    }
    delete[] digits;
    digits = newDigits;
    numDigits = newNumDigits;
};

// the hardest one
// but also the most important
BigNumber::BigNumber(std::string hexRepresentation) {
    const int hexPerDigit = BASE_BITS / 4;

    int hexLength = hexRepresentation.length();
    // we need a multiple of 8 as the length
    int lengthModded = hexLength % hexPerDigit;
    std::string hexPadded = hexRepresentation;
    if (lengthModded != 0) {
        std::string padding("");
        for (int i = lengthModded; i < hexPerDigit; i++) {
            padding += "0";
        }
        hexPadded = padding + hexRepresentation;
        hexLength = hexPadded.length();
    }
    // c strings are easier to manipulate
    const char* hexChars = hexPadded.c_str();

    if (hexLength == 0) {
        digits = new bndigit[1];
        digits[0] = 0;
        numDigits = 1;
        return;
    }
    
    // we know the number of digits now
    numDigits = hexLength / hexPerDigit;
    digits = new bndigit[numDigits];

    // d = digit index; c = char index
    int d = numDigits - 1;
    int c = 0;
    while (c < hexLength) {
        std::stringstream ss;
        ss << "1"; // leading 1 is to suppress errors
        for (int o = 0; o < hexPerDigit; o++) {
            ss << hexChars[c + o];
        }
        std::string digitInHex;
        getline(ss, digitInHex);
        int64 digit = std::stoull(digitInHex, nullptr, 16) - BASE; // some weird way of making it go for hex
        digits[d] = (bndigit)digit;

        c += hexPerDigit;
        d -= 1;
    }
};
BigNumber::BigNumber(const BigNumber& other) {
    numDigits = other.getNumDigits();
    digits = new bndigit[numDigits];
    for (int i = 0; i < numDigits; i++) {
        digits[i] = other.getDigit(i);
    }

    this->simpleDeflate();
};
BigNumber::BigNumber(BigNumber& other) {
    numDigits = other.getNumDigits();
    digits = new bndigit[numDigits];
    for (int i = 0; i < numDigits; i++) {
        digits[i] = other.getDigit(i);
    }

    this->simpleDeflate();
};
// the "move constructor" that I dont fully get
BigNumber::BigNumber(BigNumber&& other)
    : digits(nullptr), numDigits(0)
{
    numDigits = other.getNumDigits();
    digits = other.digits;
    
    // destroy resources in other
    other.digits = nullptr;
    other.numDigits = 0;

    this->simpleDeflate();
};
// assignment copy
BigNumber& BigNumber::operator=(const BigNumber& other) {
    if (digits != nullptr) {
        delete[] digits;
    };
    numDigits = other.getNumDigits();
    digits = new bndigit[numDigits];
    for (int i = 0; i < numDigits; i++) {
        digits[i] = other.getDigit(i);
    }

    this->simpleDeflate();
    return *this;
};
// the move assignment operator
BigNumber& BigNumber::operator=(BigNumber&& other) noexcept {
    if (this != &other) {
        delete[] digits;
        digits = other.digits;
        numDigits = other.numDigits;
        // set the other's stuff to zero
        other.digits = nullptr;
        other.numDigits = 0;
    }
    this->simpleDeflate();
    return *this;
};

BigNumber::~BigNumber() {
    delete[] digits;
};

// some easy methods
int BigNumber::getNumDigits() const {
    return numDigits;
};
// e.g. getDigit(0) returns the 1s digit, (1) returns the (2^16)s digit, etc
// going beyond the size of the array returns 0
bndigit BigNumber::getDigit(int pos) const {
    if (pos >= numDigits) {
        return (bndigit)0; // like how the thousands digit of 42 (0042) is 0
    } else {
        return digits[pos];
    }
};

bool BigNumber::isZero() const {
    for (int i = numDigits - 1; i >= 0; i--) {
        if (digits[i] != (bndigit)0) {
            return false; // non-zero digit found
        }
    }
    return true;
};

// deflate() copies the array into the minimum possible size
// if it somehow ends up with a lot of leading 0s this helps save heap space
void BigNumber::deflate() {
    int firstNonZero = numDigits;
    bool keepGoing = true;
    while (keepGoing) {
        firstNonZero--;
        if (digits[firstNonZero] != (bndigit)0) {
            keepGoing = false;
        }
        if (firstNonZero == 0) {
            keepGoing = false; // stop from looping into danger territory
        }
    }
    // firstNonZero is an index
    // we can safely overwrite numDigits because the only digits we leave behind are zeroes
    numDigits = firstNonZero + 1;
    // now we need a buffer
    bndigit* buffer = new bndigit[numDigits];
    // memcpy is theoretically faster
    memcpy(buffer, digits, numDigits * sizeof(bndigit));
    // we need to store the location so we can delete it
    // ah, pointer assignments... the only reason I can stand C++
    bndigit* oldDigits = digits;
    digits = buffer;
    // now we need to free up that old slot
    // we don't delete buffer because it points to digits
    delete[] oldDigits;
};

// these are late additions to my class
// mostly because I am a bit lazy with deflates
// this returns the VALUE of the most significant non-zero digit or else 0
bndigit BigNumber::getLeadingDigit() const {
    for (int i = numDigits - 1; i >= 0; i--) {
        if (digits[i] != 0) {
            return digits[i];
        }
    }
    return (bndigit)0;
};
// ...whereas this one returns the INDEX of that digit
// I feel like I should start using this instead of numDigits more often
int BigNumber::getLeadingDigitPosition() const {
    for (int i = numDigits - 1; i >= 0; i--) {
        if (digits[i] != 0) {
            return i;
        }
    }
    return 0;
};
// Like deflate() but does not actually re-allocate memory
// I wonder if deflations are slowing down complicated operations
void BigNumber::simpleDeflate() {
    numDigits = getLeadingDigitPosition() + 1;
};

BigNumber operator+(const BigNumber& a, const BigNumber& b) {
    int aDigits = a.getNumDigits();
    int bDigits = b.getNumDigits();
    // find the larger of the two numbers
    int maxDigits = (aDigits > bDigits ? aDigits : bDigits);
    // plus 1 so it carries; int so we can detect carries
    bndigit* buffer = new bndigit[maxDigits + 1];

    // Fill buffer with 0
    int i;
    for (i = 0; i < maxDigits + 1; i++) {
        buffer[i] = (bndigit)0;
    }

    // add in the digits
    for (i = 0; i < maxDigits; i++) {
        int64 colSum = (int64)(a.getDigit(i)) + (int64)(b.getDigit(i)) + (int64)(buffer[i]); // add A, B, and previous carry
        buffer[i] = (bndigit)(colSum & (BASE - 1));
        buffer[i+1] = (bndigit)(colSum >> BASE_BITS); // carry digit
    }

    // buffer is complete
    // construct the BigNumber
    BigNumber answer;
    // this way we save a copy operation
    answer.directlyAssignDigits(buffer, maxDigits + 1, 0xbe5afe);
    return answer;
};

BigNumber& BigNumber::operator+=(const BigNumber& other) {
    BigNumber sum = *this + other;
    // copy sum into this
    numDigits = sum.getNumDigits();
    delete[] digits;
    digits = new bndigit[numDigits];
    for (int i = 0; i < numDigits; i++) {
        digits[i] = sum.getDigit(i);
    }
    this->simpleDeflate();
    return *this;
};

// Multiply by a single "digit" (i.e. a number from 0 to 2^32-1)
BigNumber operator*(const BigNumber& a, bndigit b) {
    // This might save some time in big multiplications
    if (b == (bndigit)0) {
        // construct a zero
        BigNumber zero;
        return zero;
    }

    int aDigits = a.getNumDigits();
    int64 bAsInt = (int64)b;
    bndigit* buffer = new bndigit[aDigits + 1];
    int i;
    // fill buffer with 0
    for (i = 0; i < aDigits + 1; i++) {
        buffer[i] = 0;
    }
    // multiply
    for (i = 0; i < aDigits; i++) {
        // multiply this digit by the multiplier, then add the carry
        int64 product = (int64)(a.getDigit(i)) * bAsInt + (int64)(buffer[i]);
        buffer[i] = (bndigit)(product % BASE);
        // here the carry is not just 0 or 1
        // but I know it will be less than BASE
        buffer[i+1] = (bndigit)(product >> BASE_BITS);
    }

    BigNumber answer;
    answer.directlyAssignDigits(buffer, aDigits + 1, 0xbe5afe);
    return answer;
};

// places > 0 means multiply by BASE^places, < 0 means divide (truncated)
BigNumber BigNumber::getShift(int places) const {
    // if you negatively shift more places than you have you get 0
    // e.g. shift 5280 by 4 digits
    if (-places >= numDigits) {
        // hmm this is weird
        BigNumber zero;
        return zero;
    }
    
    // interestingly this method is quite symmetric
    int newDigits = numDigits + places;
    bndigit* buffer = new bndigit[newDigits];
    for (int i = 0; i < newDigits; i++) {
        int aIndex = i - places; // e.g. shift 3 means new index 5 should be old index 2
        if (aIndex < 0) {
            buffer[i] = (bndigit)0;
        } else {
            buffer[i] = (bndigit)(digits[aIndex]);
        }
    }
    BigNumber answer(buffer, newDigits);
    delete[] buffer;
    return answer;
};

// This is effectively the modulo function when the modulus is a power of 256
BigNumber BigNumber::getLowestNDigits(int n) const {
    if (n == 0) {
        BigNumber zero;
        return zero;
    } else if (n > numDigits) {
        return *this; // should make a copy of this
    } else {
        // use our digits array in the array constructor
        // the constructor copies only as many numbers as the second parameter!
        return BigNumber(digits, n);
    }
};

// Uses the Karatsuba algorithm
// https://en.wikipedia.org/wiki/Karatsuba_algorithm

// I timed this on a few scenarios and 200 worked best.
// I guess the K. method is rather inefficient for small sizes.
// A bit like the bubble sort.
#define MULT_CUTOFF 200
BigNumber operator*(const BigNumber& a, const BigNumber& b) {
    int aDigits = a.getNumDigits();
    int bDigits = b.getNumDigits();

    if (aDigits >= MULT_CUTOFF && bDigits >= MULT_CUTOFF) {
        // Use the K. algorithm
        // write a = a1*M + a0, b = b1*M + b0, where M is a power of 256
        int minDigits = (aDigits < bDigits) ? aDigits : bDigits;
        int N = (minDigits + 1) / 2;

        BigNumber a1 = a.getShift(-N);
        BigNumber a0 = a.getLowestNDigits(N);
        BigNumber b1 = b.getShift(-N);
        BigNumber b0 = b.getLowestNDigits(N);

        BigNumber z2 = a1 * b1; // recursion!
        BigNumber z0 = a0 * b0; // recursion again!
        BigNumber z1 = (a1 + a0) * (b1 + b0) - (z2 + z0); // third recursion!

        return z2.getShift(N*2) + z1.getShift(N) + z0;
    } else {
        int ansDigits = aDigits + bDigits;
        bndigit* buffer = new bndigit[ansDigits];
        // 32 bit by 32 bit can sometimes carry twice.
        int64 colSum = 0;
        int64 carry = 0;
        // loop to including (aDigits - 1) + (bDigits - 1)
        int loopBound = aDigits + bDigits - 2;
        for (int i = 0; i <= loopBound; i++) {
            // colSum is not reset; we add on to whatever carried from last time
            // anyway, we need these digit indices
            int ca = i;
            int cb = 0;
            if (ca >= aDigits) {
                // ensure they are all in range (avoids wasting steps)
                cb = ca - (aDigits - 1);
                ca = aDigits - 1;
            }
            // ca only goes down, cb only goes up
            while (ca >= 0 && cb < bDigits) {
                colSum += (int64)(a.getDigit(ca)) * (int64)(b.getDigit(cb));
                // because we only have enough room for 1 multiplication
                // we have to deposit carries immediately
                carry += (colSum >> (int64)BASE_BITS);
                colSum = colSum & (BASE - 1ull);
                ca--;
                cb++;
            }

            // put the answer into the buffer
            buffer[i] = (bndigit)colSum;
            // Delete those bits and carry anything else
            colSum = carry % BASE;
            // meta-carries may happen in the 32-bit version
            carry = carry >> BASE_BITS;
        }

        if (colSum >= BASE) {
            // houston, we have a problem
            std::cerr << "Multiplication Panic: Too much to carry!" << std::endl;
        }
        // e.g. 2 digits by 2 digits uses at most 4 digits
        // so put the final carry in slot (2+2)-1 = 3
        buffer[aDigits + bDigits - 1] = (bndigit)colSum;
        BigNumber answer(buffer, ansDigits);
        delete[] buffer;
        return answer;
    }
};

// I find I am copying a lot of code around to make stuff work
BigNumber& BigNumber::operator*=(const BigNumber& other) {
    BigNumber prod = *this * other;
    // copy prod into this
    numDigits = prod.getNumDigits();
    delete[] digits;
    digits = new bndigit[numDigits];
    for (int i = 0; i < numDigits; i++) {
        digits[i] = prod.getDigit(i);
    }
    return *this;
};

// I am not sure what happens to this function
int compare(const BigNumber& a, const BigNumber& b) {
    int aDigits = a.getNumDigits();
    int bDigits = b.getNumDigits();
    int maxDigits = (aDigits > bDigits) ? aDigits : bDigits;
    // scan most significant digit first
    for (int i = maxDigits - 1; i >= 0; i--) {
        bndigit aDigit = a.getDigit(i);
        bndigit bDigit = b.getDigit(i);
        if (aDigit < bDigit) {
            // a is smaller
            return -1;
        } else if (aDigit > bDigit) {
            return +1;
        }
    }
    
    return 0;
};

// Compare can now be used to overload operators
bool operator<(const BigNumber& a, const BigNumber& b) {
    int result = compare(a, b);
    return result == -1;
};
bool operator<=(const BigNumber& a, const BigNumber& b) {
    int result = compare(a, b);
    return result <= 0;
};
bool operator>(const BigNumber& a, const BigNumber& b) {
    int result = compare(a, b);
    return result == 1;
};
bool operator>=(const BigNumber& a, const BigNumber& b) {
    int result = compare(a, b);
    return result >= 0;
};
bool operator==(const BigNumber& a, const BigNumber& b) {
    int result = compare(a, b);
    return result == 0;
};
bool operator!=(const BigNumber& a, const BigNumber& b) {
    int result = compare(a, b);
    return result != 0;
};

// Here comes subtraction!
BigNumber operator-(const BigNumber& a, const BigNumber& b) {
    if (a < b) {
        // some debug info
        std::cerr << "Panic in BigNumber - BigNumber" << std::endl;
        // I originally called this function "dumpDigits" because it reminded me of the expression "Segmentation fault (core **dumped**)"
        a.dumpDigits();
        b.dumpDigits();
        throw std::string("Error in a - b: a cannot be smaller");
    }
    
    // if a >= b then a must have at least as many digits as b
    int aDigits = a.getNumDigits();
    bndigit* buffer = new bndigit[aDigits];
    
    // do the subtraction!
    // I have to use "long long" in order to handle negatives
    long long borrow = 0; // set to -1 if appropriate
    for (int i = 0; i < aDigits; i++) {
        long long aDigit = (long long)(a.getDigit(i));
        long long bDigit = (long long)(b.getDigit(i));
        long long column = aDigit - bDigit + borrow;
        if (column >= 0) {
            buffer[i] = (bndigit)column;
            borrow = 0ll;
        } else {
            // no subtraction can need to borrow more than 1 from the next column
            borrow = -1ll;
            buffer[i] = (bndigit)(column + BASE);
        }
    }

    // now we have to construct the answer
    // but first a sanity check
    if (borrow != 0) {
        std::cerr << "Double panic in BigNumber - BigNumber" << std::endl;
        a.dumpDigits();
        b.dumpDigits();
        throw std::string("Error in a - b: borrow escaped by accident!");
    }

    BigNumber answer;
    answer.directlyAssignDigits(buffer, aDigits, 0xbe5afe);
    // now before we return the answer we have to deflate it
    // because otherwise something like (1,1,168,192) - (2,0,168,192) will return (255,0,0,0) which when combined with other operations those leading zeros will grow rapidly
    answer.simpleDeflate();
    return answer;
};

// I find I am copying a lot of code around to make stuff work
BigNumber& BigNumber::operator-=(const BigNumber& other) {
    BigNumber diff = *this - other;
    // copy diff into this
    numDigits = diff.getNumDigits();
    delete[] digits;
    digits = new bndigit[numDigits];
    for (int i = 0; i < numDigits; i++) {
        digits[i] = diff.getDigit(i);
    }
    return *this;
};

// Helper function for the long division.
// Note that you MUST ensure that the denominator is correctly lined up or else it will throw.
bndigit singleDigitDivide(BigNumber& a, BigNumber& b) {
    // spare a bunch of multiplies if a is smaller than b
    if (a < b) {
        return (bndigit)0;
    }
    // Try to make an educated guess.
    // This uses the leading digits of a and b.
    int64 ald = (int64)a.getLeadingDigit();
    int64 bld = (int64)b.getLeadingDigit();
    int aldp = a.getLeadingDigitPosition();
    int bldp = b.getLeadingDigitPosition();
    
    // Initial guesses.
    int64 low = 0ull;
    int64 high = BASE - 1ull;

    // Refine our guesses.
    if (aldp == bldp) {
        // Scenario 1: Digits line up
        // e.g. 468 / 212
        // so it must be between 500/200 and 400/300, or 5/2 and 4/3
        low = ald / (bld + 1ull);
        high = (ald + 1ull) / bld;
    } else if (aldp == bldp + 1) {
        // Scenario 2: Digits are offset by 1
        // e.g. 468 / 77
        // so it must be between 500/70 and 400/80
        // which are (5 * 10) / 7 and (4 * 10) / 8.
        low = (ald * BASE) / (bld + 1ull);
        high = ((ald + 1ull) * BASE) / bld;
    } else {
        a.dumpDigits();
        b.dumpDigits();
        throw std::string("Oops! Single digit division is not lined up; bad!");
    }

    // bound check
    // high > BASE can happen with a division like 299 / 31
    if (high > BASE - 1ull) {
        high = BASE - 1ull;
    }

    // if we are so lucky that...
    if (low == high) {
        return (bndigit)low;
    }

    // basically, binary search.
    int time = 40;
    //std::cout << "Starting..." << std::endl;
    while (low < high) {
        //std::cout << low << " " << high << std::endl;
        int64 guess = (low + high + 1ull)/2ull;
        // 1 = a bigger, -1 = a smaller, 0 = equal
        int result = compare(a, b * (bndigit)guess);
        if (result == 1) {
            // a is bigger; guess may be too small or just right
            low = guess;
        } else if (result == -1) {
            // b*guess is bigger, so guess is definitely too big
            high = guess - 1ull;
        } else {
            // we hit it just right
            // no need to keep searching
            return (bndigit)guess;
        }
        if (!time--) throw 0;
    }

    return (bndigit)low;
};

// here it is... long division!!!
BigNumber doLongDivision(const BigNumber& a, const BigNumber& b, bool wantQuotient) {
    if (b.isZero()) {
        throw std::string("Error: BigNumber division by zero");
    }
    // Step 1: deflate them
    // ...which involves making copies
    BigNumber aCopy = a;
    BigNumber bCopy = b;
    aCopy.simpleDeflate();
    bCopy.simpleDeflate();
    // Step 2: Check if we are already done
    if (aCopy < bCopy) {
        // I hate this
        if (wantQuotient) {
            BigNumber zero;
            return zero;
        } else {
            return aCopy;
        }
    }
    // Step 3: Setup for the loop
    int aDigits = aCopy.getNumDigits();
    int bDigits = bCopy.getNumDigits();
    // e.g. 5 digits / 3 digits will be either A-B=2 or A-B+1=3 digits
    bndigit* buffer = new bndigit[aDigits - bDigits + 1];
    // I can actually mess up aCopy because it is a copy of a
    for (int i = (aDigits - bDigits); i >= 0; i--) {
        // Initially this shifts b to line up with a
        BigNumber bShifted = bCopy.getShift(i);
        bndigit quotient = singleDigitDivide(aCopy, bShifted);
        // I really hope this does not degenerate to all 255s
        aCopy -= (bShifted * quotient);
        buffer[i] = quotient;
    }
    if (wantQuotient) {
        BigNumber answer;
        answer.directlyAssignDigits(buffer, aDigits - bDigits + 1, 0xbe5afe);
        answer.simpleDeflate();
        return answer;
    } else {
        // return the remainder instead
        delete[] buffer;
        return aCopy;
    }
};

BigNumber operator/(const BigNumber& a, const BigNumber& b) {
    return doLongDivision(a, b, true);
};
BigNumber operator%(const BigNumber& a, const BigNumber& b) {
    return doLongDivision(a, b, false);
};

// I find I am copying a lot of code around to make stuff work
BigNumber& BigNumber::operator/=(const BigNumber& other) {
    BigNumber quot = *this / other;
    // copy diff into this
    numDigits = quot.getNumDigits();
    delete[] digits;
    digits = new bndigit[numDigits];
    for (int i = 0; i < numDigits; i++) {
        digits[i] = quot.getDigit(i);
    }
    return *this;
};

// modular exponentiation: modExp(a,b,c) returns a^b mod c.
// basic idea:
// say we need the last 3 digits of 7^42
// we would compute 7, 7^2, 7^4, ..., 7^32, and multiply 7^32, 7^8, and 7^2.
// even better, we can take the mod function at each step.
// so our third number is 7^4
BigNumber BigNumber::modExp(const BigNumber& base, const BigNumber& exp, const BigNumber& modulus) {
    // immediate sanity checks
    if (modulus.isZero()) {
        throw std::string("BigNumber::modExp() modulus is zero");
    } else if (exp.isZero()) {
        return BigNumber(1);
    } else if (base.isZero()) {
        return BigNumber(0);
    }
    int expBits = exp.getNumDigits() * BASE_BITS;
    BigNumber product(1);
    BigNumber multiplier(base);

    for (int i = 0; i < expBits; i++) {
        // (1 << x) is fancy for 2 to the x power
        // & operator here is a bitwise and
        bndigit bitI = exp.getDigit(i / BASE_BITS) & (1 << (i % BASE_BITS));
        if (bitI != (bndigit)0) {
            // multiply by the current power of the base
            product = (product * multiplier) % modulus;
        }
        // if last time we had b^n, here we create b^(2n)
        // but always reduce it mod the modulus!
        multiplier = (multiplier * multiplier) % modulus;
    }
    return product;
};

// standard Euclidean algorithm
BigNumber BigNumber::gcd(BigNumber& a, BigNumber& b) {
    bool aIsBigger = (a > b);
    BigNumber higher = aIsBigger ? a : b;
    BigNumber lower = aIsBigger ? b : a;
    while (!lower.isZero()) {
        // (h, l) become (l, h % l)
        BigNumber temp = higher % lower;
        higher = lower;
        lower = temp;
    }
    return higher;
};

// Filters out any number divisible by 2, 3, 5, 7, 11, or 13.
// Among odd numbers, only 2/3 * 4/5 * 6/7 * 10/11 * 12/13,
// or about 38%, of them will survive this stage.
bool BigNumber::couldBePrime() {
    // 2 is legitimately trivial
    if (digits[0] % 2 == 0) {
        return false;
    }
    // check all primes under 1000
    int numPrimes = 167;
    int primes[] = {
3, 5, 7, 11, 13, 17, 19, 23, 
29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 
71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 
113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 
173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 
229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 
281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 
349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 
409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 
463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 
541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 
601, 607, 613, 617, 619, 631, 641, 643, 647, 653, 
659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 
733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 
809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 
863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 
941, 947, 953, 967, 971, 977, 983, 991, 997 
    };
    for (int i = 0; i < numPrimes; i++) {
        BigNumber num(primes[i]);
        // read this as "n % 3 == 0" or the like
        if ( (*this % num).isZero() ) {
            return false;
        }
    }
    // passed all our simple tests
    return true;
};

// Tests if the number is prime. Uses a Fermat and Miller-Rabin primality test.
bool BigNumber::fermatPrimalityTest() {
    // The base of 2 seems to be quite common around here
    BigNumber a(2);

    // Fermat test
    // a^p should be congruent to a, modulo p
    // this is not 100% reliable tho
    BigNumber result = BigNumber::modExp(a, *this, *this);
    std::cout << result << std::endl;
    return result == a;
};

// true means probably prime but you can't know for sure
bool BigNumber::millerRabinTest() {
    // Check for trivial cases
    if (*this < BigNumber(2)) {
        return false;
    }
    // Get our first digit
    bndigit firstDigit = getDigit(0);
    // number is even
    if (firstDigit % 2 == 0) {
        return false;
    }
    if (firstDigit == (bndigit)1) {
        // in binary we have something like ...1000...0001
        // with 31 or more 0's.
        // I don't like that, so I just send that number packing. Try again!
        return false;
    }

    // it might be nice to have a copy of n-1
    BigNumber nMinus1 = *this - 1;
    
    // Get how many times we can divide (n-1) by 2
    int factor2s = 0;
    // We can burn the firstDigit variable
    firstDigit -= 1;
    while (firstDigit % 2 == 0) {
        firstDigit /= 2;
        factor2s++;
    }
    // 1ull makes sure we get a 64-bit int
    BigNumber powerOfTwo(1ull << factor2s);
    BigNumber d = *this / powerOfTwo;

    // Time for the real test!
    // We will see how well this times...
    for (int trials = 0; trials < 8; trials++) {
        BigNumber test(rand());
        BigNumber x = BigNumber::modExp(test, d, *this);
        if (x == BigNumber(1)) {
            continue; // number is probably prime (or test is a strong liar)
        }
        if (x == nMinus1) {
            continue; // ditto
        }
        // repeatedly square x and see if it reaches 1
        for (int i = 1; i <= (factor2s - 1); i++) {
            x = (x * x) % (*this);
            if (x == nMinus1) {
                continue; // yet another prime testimony
            }
        }
        // If none of the continues were hit, then we found a guilty witness!
        return false;
    }

    // Otherwise, the number is probably prime.
    return true;
};
bool BigNumber::isProbablyPrime() {
    return couldBePrime() && millerRabinTest();
};

// hashes probably are not proper ways to generate random numbers
// but I don't really have a choice
BigNumber BigNumber::generateLikelyPrime(int bits, Randomizer& generator) {
    int hexDigits = (bits + 2) / 4;
    bool keepGoing = true;
    int tries = 0;
    int hopefuls = 0;
    // generate the first random number
    // end with a 1 to ensure number is odd
    std::string randomDigits = generator.randomHex(hexDigits - 1) + std::string("1");
    BigNumber primeGuess(randomDigits);
    while (keepGoing) {
        tries++;
        // first check for easy factors (2 to 997)
        if (primeGuess.couldBePrime()) {
            hopefuls++;
            // if we get here, run a Miller-Rabin
            if (primeGuess.millerRabinTest()) {
                // it's probably prime.
                return primeGuess;
            }
            // are they failing at this step?
            if (hopefuls % 10 == 0) {
                //std::cout << "That is the " << hopefuls << "th time a promising almost-prime has been shot down." << std::endl;
            }
        }
        // fail messages
        if (tries % 200 == 0) {
            //std::cout << "0x" << randomDigits << "," << std::endl;
            //std::cout << "Found " << tries << " ways not to get a prime..." << std::endl;
        }
        // update the number
        primeGuess += BigNumber(2);
    }
    // to satisfy the compiler
    return primeGuess;
};


// It is fitting for this to be the last function. That, or first, but that part of the file is messed up, so...
std::string BigNumber::convertToHex() const {
    int hexPerDigit = BASE_BITS / 4;
    int charSize = numDigits * hexPerDigit;
    char* buffer = new char[charSize];
    for (int i = 0; i < numDigits; i++) {
        int64 digitValue = (int64)digits[i];
        for (int j = 0; j < hexPerDigit; j++) {
            int chop = digitValue & 0xf; // just enough for 1 hex digit!
            char hexChar;
            if (chop < 10) {
                // digits are stored 48-57
                hexChar = (char)(chop + 48);
            } else {
                // letters are stored with a = 97
                hexChar = (char)(chop + 97 - 10);
            }
            buffer[charSize - 1 - j - hexPerDigit*i] = hexChar;
            // actually remove that digit from consideration now
            digitValue = digitValue / 16ull;
        }
    }
    std::string result(buffer, charSize);
    delete[] buffer;
    return result;
};

std::ostream& operator<<(std::ostream& os, const BigNumber& a) {
    os << a.convertToHex();
    return os;
};





// Testing functions
void BigNumber::dumpDigits() const {
    std::cout << "" << numDigits << ": {";
    for (int i = 0; i < numDigits; i++) {
        std::cout << (int64)digits[i]; // make it not print it as a char
        if (i < numDigits - 1) { // no trailing comma on final digit
            std::cout << ", ";
        }
    }
    std::cout << "}" << std::endl;
};

void BigNumber::test() {
    std::cout << "Testing BigNumber..." << std::endl;
    
    BigNumber n1(42);
    BigNumber n2(21);
    BigNumber n3(BASE - 20); // to make carries easy
    BigNumber n4(std::string("432b2b")); // hmmm I wonder what this is?
    BigNumber n5("feedbac0");
    BigNumber n6("c071d1915badbadbad");
    n6.dumpDigits();
    std::cout << n6.convertToHex() << std::endl;

    std::cout << "Multiplication tests" << std::endl;
    
    // multiplication of two BigNumbers
    // hmmm I need a smaller number!
    BigNumber n0(2);
    
    BigNumber mm1 = n0 * n1;
    BigNumber mm2 = n1 * n2;
    // we have to confirm commutativity
    BigNumber mm3a = n4 * n0;
    BigNumber mm3b = n0 * n4;
    BigNumber mm4a = n5 * n6;
    BigNumber mm4b = n6 * n5;
    
    mm1.dumpDigits();
    mm2.dumpDigits();
    std::cout << "Commutativity tests" << std::endl;
    std::cout << mm3a.convertToHex() << std::endl
        << mm3b.convertToHex() << std::endl
        << mm4a.convertToHex() << std::endl
        << mm4b.convertToHex() << std::endl;

    BigNumber maxOutCarry("fffffffffffffffffffffffffffffffffffffffffff");
    std::cout << "Supertest: " << (maxOutCarry * maxOutCarry) << std::endl;
    // testing comparisons
    BigNumber n7("ffff00000000");
    BigNumber n8("0000ffffffff");
    BigNumber n9("ffffffff");
    BigNumber n10("444444333333222222111111");
    BigNumber n11("111111222222333333444444");
    BigNumber n12("111111222222333333444445");

    // here we do not use dumpDigits()
    std::cout << "n7 vs n8: " << compare(n7, n8) << std::endl;
    std::cout << "n8 vs n9: " << compare(n8, n9) << std::endl;
    std::cout << "n8 vs n7: " << compare(n8, n7) << std::endl;
    std::cout << "n10 vs n11: " << compare(n10, n11) << std::endl;
    
    BigNumber s1 = n10 - n11;
    BigNumber s2 = n8 - n9;
    BigNumber s3 = n6 - n5;
    BigNumber s4 = n12 - n11;
    std::cout << "Subtraction tests" << std::endl;
    std::cout << s1 << std::endl << s2 << std::endl << s3 << std::endl << s4 << std::endl;
    s1.dumpDigits(); // expecting (237, 48) I think
    s2.dumpDigits(); // expecting only a 0
    s3.dumpDigits(); // confirmed
    s4.dumpDigits(); // expecting only a 1
    /*std::cout << "The following may be unsafe..." << std::endl;
    try {
        BigNumber fail1 = n11 - n12;
        fail1.dumpDigits();
    } catch (std::string error) {
        std::cout << "Caught error: " << error << std::endl;
    }*/

    // Division tests
    // note that we have to line the digits up ourselves
    // also this does not compute remainders
    BigNumber n13("000600000009");
    BigNumber n14("000200000002"); // q3 r3
    BigNumber n15("000200000003"); // q3 r0
    BigNumber n16("000200000004"); // q2 r253
    std::cout << "Division tests:" << std::endl
        << (int)singleDigitDivide(n13, n14) << std::endl
        << (int)singleDigitDivide(n13, n15) << std::endl
        << (int)singleDigitDivide(n13, n16) << std::endl;
    
    // something more involved (expect 193)
    //std::cout << (int)singleDigitDivide(n6, n5) << std::endl;

    // wrong order
    std::cout << (int)singleDigitDivide(n14, n15) << std::endl
        << (int)singleDigitDivide(n15, n15) << std::endl
        << (int)singleDigitDivide(n15, n14) << std::endl;
    //if(true)return;
    std::cout << "Long division tests:" << std::endl;
    // I am not sure what the worst possible division is...
    // probably 2N digits over N digits?
    BigNumber d1 = n1 / n2; // 2
    d1.dumpDigits();
    BigNumber d2 = n3 / n2; // 11
    d2.dumpDigits();
    BigNumber d3 = n3 / n3; // 1
    d3.dumpDigits();
    BigNumber d4 = n2 / n1; // 0
    d4.dumpDigits();
    BigNumber d5 = n4 / n0; // 149, 149, 33
    d5.dumpDigits();
    
    std::cout << "Harder long division" << std::endl;
    // I'm not entirely sure how to verify these
    BigNumber d6 = n6 / n4;
    BigNumber d7 = n6 / n6; // ok this one I can verify
    BigNumber d8 = n7 / n9; 
    d6.dumpDigits();
    d7.dumpDigits();
    d8.dumpDigits();
    
    
    // r for remainders
    std::cout << "Modulo tests" << std::endl;
    BigNumber r1 = BigNumber(234897) % BigNumber(8472);
    BigNumber r2 = n6 % n4;
    BigNumber r3 = n4 % n6;
    r1.dumpDigits();
    r2.dumpDigits();
    r3.dumpDigits();

    std::cout << "GCD tests" << std::endl;
    BigNumber g1 = BigNumber::gcd(n6, n4);
    BigNumber g2 = BigNumber::gcd(n4, n6);
    BigNumber g3 = BigNumber::gcd(n13, n15);
    BigNumber g4 = BigNumber::gcd(maxOutCarry, n10);
    std::cout << g1 << std::endl
              << g2 << std::endl
              << g3 << std::endl
              << g4 << std::endl << std::endl;

    // test modExp
    std::cout << "Modular exponentiation tests" << std::endl;
    BigNumber smallB(3);
    BigNumber smallE(42);
    BigNumber smallM(11);
    BigNumber smallModExp = BigNumber::modExp(smallB, smallE, smallM);
    smallModExp.dumpDigits();
/*
    // ok that was too easy
    BigNumber prime1(72421);
    BigNumber prime2(360979);
    BigNumber mediumE(329);
    BigNumber mediumX(42);
    BigNumber mediumD("155005139"); // yes this is hexadecimal
    BigNumber mediumN = prime1 * prime2;

    BigNumber mediumCipher = BigNumber::modExp(mediumX, mediumE, mediumN);
    mediumCipher.dumpDigits();
    BigNumber mediumDecode = BigNumber::modExp(mediumCipher, mediumD, mediumN);
    std::cout << mediumDecode << std::endl;
    mediumDecode.dumpDigits();
    
    // "Easy composite" test
    BigNumber easyYes1(1009); // first prime > 1000
    BigNumber easyYes2("c39962486ce3ba88788716e9ac581c5"); // prime, ~2^128
    BigNumber easyNo1(42);
    BigNumber easyNo2 = BigNumber(229) * easyYes2;
    BigNumber easyFake = easyYes1 * easyYes2; // not prime, but the easy test should mark it possibly prime
    
    std::cout << "Easy primality tests: " << std::endl
        << easyYes1.couldBePrime() << easyYes2.couldBePrime()
        << easyNo1.couldBePrime() << easyNo2.couldBePrime()
        << easyFake.couldBePrime() << " (expect: 11001) " << std::endl;

    // Fermat primality test
    BigNumber fermatYes(233);
    BigNumber fermatNo(305);
    BigNumber fermatFake(341); // 341 fools the Fermat test
    std::cout << "233: " << fermatYes.fermatPrimalityTest() << std::endl
        << "305: " << fermatNo.fermatPrimalityTest() << std::endl
        << "341 (not prime but appears so): " << fermatFake.fermatPrimalityTest() << std::endl;
*/    

    // this is a real RSA key I generated with another program
    // I don't use it for anything
    // note that D < N.
    BigNumber rsaD(
            "c4e8f8e4eea35585146ff06c5e13ad39f9673cdc8ce8eb44c11"
            "f1199a94540c6c5f2ab8293eb5b6a3e0fa2787661af65e42906"
            "9214f673fa6a2c85f4714ed84d98df20d63f30859f5c09b89f3"
            "b12ca8cdecc4cff49aed6796485d0ab72c1003bdea1bf55452d"
            "5138dcf7b81daace4bd374cb2c0825702b5d299c77dd1d04ed4"
            "210aa07995c3c0d7e75466c1e84c6180c3c88596231f57486ae"
            "5fc3d5056a690e2a0f89fd82bc149d801f34804738af71ff2ed"
            "b48d278895007b441");
    BigNumber rsaN(
            "c7f6da11bcd1d7db9bcda015a304d6325b55fe2ca8368d4583e"
            "f311062e129967a881507644dbf07cffe5a1806b456385fc504"
            "5403b70bf79d65c5ad23964cb8206b5d1909d52d89c3cdf11ed"
            "41e9d35c35c4c55c5ba80aee6326a1fd06dfcbdcdd6a0bbb8a9"
            "4878e6e2d34f6caef668132d2fb89d93a84825f1c71d6d34bf5"
            "023050dc1de084a09c68e314871d4319b6d9c41fbbab3111d9a"
            "75a8ab85da3f7104f2eca9937b2ea498f13584d6dfa074b6f8b"
            "9779f7727824066db");

    BigNumber rsaE(65537);
    // testing a long message (
    BigNumber rsaMessage(
            "00000000000000000000000000000000"
            "0123456789abcdeffedcba9876543210"
            "32107654ba98fedc0f1e2d3c4b5a6978"
            "00112233445566778899aabbccddeeff"
            "13579bdf02468acefdb97531eca86420"
            "73615736238746237462378467382647"
            "faceb00715badbad123412341234eeee"
            );

    
    // tests for large mod-exp
    /*
    const int NUM_DIGITS = 500;
    uint16_t arr1[NUM_DIGITS * 2];
    uint16_t arr2[NUM_DIGITS];
    for (int i = 0; i < NUM_DIGITS * 2; i++) {
        arr1[i] = (uint16_t)(rand() & 0xff);
    }
    for (int i = 0; i < NUM_DIGITS; i++) {
        arr2[i] = (uint16_t)(rand() & 0xff);
    }
    BigNumber huge(arr1, NUM_DIGITS * 2);
    BigNumber huge2(arr2, NUM_DIGITS);*/

    // testing large number on lots of small primes
    BigNumber bigPrime("7b71b6cc303fcc6a63f7ee02a9f34f332dc86b11450eaed1dc"
                       "c565befafcd5797df00a7c6944edb379542190daea6f1a2cad"
                       "7825830f2813d24af55b67dff");
    BigNumber bigFactor1("2006ab550c50c01a00bfd0faf5b516392f4c28ca8836abccb6"
                         "26dd8ed38bfd9");
    BigNumber bigFactor2("22c8eb52221ae4b033c12fb00763ff2b698509985e393bf41c"
                         "e33e508898409");

    BigNumber bigComposite = bigFactor1 * bigFactor2;
    
    /*Randomizer myGenerator("72983rh23RA#hq8332#(ufwefewEW{Ggf<G>Sjfei.");
    for (int i = 0; i < 100; i++) {
        BigNumber p = BigNumber::generateLikelyPrime(500, myGenerator);
        std::cout << "0x" << p << ", \\" << std::endl;
    }*/


    std::cout << "Timer Test Upcoming!!" << std::endl << std::endl;

    auto start = std::chrono::steady_clock::now();
    // START OF TIMER !!!
/*
    for (int i = 0; i < 10; i++) {
        BigNumber a = huge / huge2;
    }
*/
    BigNumber cipher = BigNumber::modExp(rsaMessage, rsaE, rsaN);
    BigNumber decoded = BigNumber::modExp(cipher, rsaD, rsaN);
    std::cout << "Decrypted as " << decoded << std::endl;

    //std::cout << "Large primality test: " << rsaN.fermatPrimalityTest() << std::endl;
    
    
    // Test the speed of my primality test
    //std::cout << (bigComposite.isProbablyPrime() ? "prime" : "composite") << std::endl;
    
    // Test generating a random prime of size 500 bits
    
    Randomizer gen("2837m8923hqw9r8y23");
    for (int i = 0; i < 10; i++) {
        //BigNumber maybePrime = BigNumber::generateLikelyPrime(500, gen);
    }
    //std::cout << maybePrime << " could be prime?" << std::endl;
    
    // END OF TIMER !!!
    // this from cppreference.com example
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "The operation took " << diff.count() << "s" << std::endl;
    std::cout << (rsaN.getNumDigits() * BASE_BITS) << " bits" << std::endl;

    //cipher.dumpDigits();
    //decoded.dumpDigits();
};
