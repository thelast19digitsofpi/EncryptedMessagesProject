/*
 * randomizer.h
 * Croix Gyurek
 *
 * My best try at making things random enough.
 * All I have access to in a console application is user input and time.
 * And hashing.
 * I know, I know, it's not good security, but give me a break, I have to code everything by hand in C++!
 */

#ifndef RANDOMIZER_H_EXISTS
#define RANDOMIZER_H_EXISTS

#include <string>
#include <sstream>
#include "sha.h"
// time is of the essence
#include <chrono>

class Randomizer {
    protected:
        std::string seedData;
        int cursor;
        std::stringstream digitQueue;
        
        // this function is protected, why not?
        void getMoreBytes();
    public:
        Randomizer();
        Randomizer(Randomizer&);
        Randomizer(std::string);

        uint64_t getTime();
        void giveExtraStuff(std::string);
        void insertTime();
        std::string randomHex(int);
        void compact();
};

void testRandomizer();

#endif

