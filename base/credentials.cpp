/*
 * credentials.cpp
 * Croix Gyurek
 *
 * Access granted
 */


#include "credentials.h"
#include "sha.h"

Credentials::Credentials() {
    username = "";
    salt = "";
    hashedPassword = "";
};

Credentials::Credentials(std::string givenName, std::string givenSalt, std::string password) {
    username = givenName;
    salt = givenSalt;
    
    // hash the password
    hashedPassword = sha256(username + salt + password);
};

std::string Credentials::getUsername() {
    return username;
};
std::string Credentials::getSalt() {
    return salt;
};
std::string Credentials::getHash() {
    return hashedPassword;
};

bool Credentials::checkLogin(std::string usernameGuess, std::string passwordGuess) {
    if (usernameGuess != username) {
        return false;
    }

    // hash username + salt + password
    std::string hashCheck = sha256(username + salt + passwordGuess);
    return hashCheck == hashedPassword;
};

// Returns true if successful.
bool Credentials::changePassword(std::string oldPassword, std::string newPassword) {
    if (checkLogin(username, oldPassword)) {
        // correctly guessed old password
        hashedPassword = sha256(username + salt + newPassword);
        return true;
    } else {
        // You failed.
        return false;
    }
};

