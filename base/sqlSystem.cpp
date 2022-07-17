/*
 * system.cpp
 * Croix Gyurek
 *
 * Maybe Java has gotten to me a little bit.
 * I think most C programs would have this be main.cpp.
 */

#include <iostream>

#include "sqlSystem.h"

SQLSystem::~SQLSystem() {
    sqlite3_close(database);
};

void SQLSystem::init(std::string databaseFile) {
    //char* errorMessage = nullptr;
    int errorCode = 0;

    errorCode = sqlite3_open(databaseFile.c_str(), &database);
    if (errorCode) {
        std::cout << "Error: Could not open the database!" << std::endl;
        exit(1);
    }
};

