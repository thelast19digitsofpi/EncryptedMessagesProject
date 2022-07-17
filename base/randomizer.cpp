/*
 * randomizer.cpp
 * Croix Gyurek
 *
 * nothing is really truly random... or is it?
 */

#include "randomizer.h"
#include <sstream>

uint64_t Randomizer::getTime() {
    auto present = std::chrono::high_resolution_clock::now();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(present.time_since_epoch());
    return nanos.count();
};

Randomizer::Randomizer() {
    // use another stringstream to get an initial seed
    std::stringstream ss;
    ss << getTime();
    getline(ss, seedData);
    // clear the digit queue
    digitQueue.str(std::string());
    digitQueue.clear();
};

Randomizer::Randomizer(Randomizer& other) {
    // copy it all over
    seedData = other.seedData;
    cursor = other.cursor;
    digitQueue.str(std::string());
    digitQueue.clear();
    digitQueue << other.digitQueue.str();
};

Randomizer::Randomizer(std::string seed) {
    std::stringstream ss;
    ss << getTime() << ';' << seed;
    seedData = ss.str();
    cursor = 0;
    getMoreBytes(); // makes it ready to use
};

// Methods for adding more entropy to the generator
void Randomizer::giveExtraStuff(std::string moreStuff) {
    seedData = seedData + moreStuff;
};
void Randomizer::insertTime() {
    seedData = seedData + std::to_string(getTime());
};

// internal function to reset the byte stack
void Randomizer::getMoreBytes() {
    // put the seed data and the current time into the hash algorithm
    std::string newData = sha256(seedData + std::to_string(getTime()));
    digitQueue.str(newData);
    digitQueue.clear();
};

std::string Randomizer::randomHex(int digits) {
    // Read off n digits into a string, how hard is that?
    std::stringstream answer;
    for (int i = 0; i < digits; i++) {
        // Check if stream is empty
        if (digitQueue.rdbuf()->in_avail() > 0 ||
                digitQueue.str().empty() ||
                digitQueue.eof()) {
            getMoreBytes();
        }
        // Try to read 1 character
        char c;
        digitQueue >> c;
        answer << c;
    }
    return answer.str();
};

void Randomizer::compact() {
    // used to tame the length of the string
    seedData = sha256(seedData + std::to_string(getTime()));
};

#include <iostream>
void testRandomizer() {
    Randomizer r1("johndoe42;832742m98m2983hfq3RWA#@#AG${}<>aUTIWhwoya4y729");
    std::cout << r1.randomHex(8) << std::endl;
    // more involved generation
    std::cout << r1.randomHex(40) << std::endl << r1.randomHex(100) << std::endl;
    r1.compact();
    std::cout << r1.randomHex(5) << std::endl;
};

