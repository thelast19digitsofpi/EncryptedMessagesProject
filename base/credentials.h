/*
 * credentials.h
 * Croix Gyurek
 *
 * You do not have permission to do that.
 */

#ifndef CREDENTIALS_H_EXISTS
#define CREDENTIALS_H_EXISTS

#include <string>

class Credentials {
    protected:
        std::string username;
        std::string salt;
        std::string hashedPassword;
    public:
        Credentials();
        Credentials(std::string, std::string, std::string);
        
        std::string getUsername();
        std::string getSalt();
        std::string getHash();
        bool checkLogin(std::string, std::string);
        bool changePassword(std::string, std::string);
};

#endif
