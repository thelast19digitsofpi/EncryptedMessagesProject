/*
 * sha256.h
 * Croix Gyurek
 *
 * all and only what you would expect
 * one function is all you need
 *
 * result returned in hexadecimal
 */

#ifndef SHA_H_EXISTS
#define SHA_H_EXISTS

#include <string>
#include <sstream>
#include <iomanip>

// I like doing this for anything I know is not a firm solid "int"
typedef uint32_t shaInt;
typedef uint8_t shaByte;

//std::string sha256(shaByte*, int); this one is broken (gives wrong hashes)
std::string sha256(std::string);

void testSHA();

#endif

