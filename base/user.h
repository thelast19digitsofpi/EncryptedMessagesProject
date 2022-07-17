/*
 * user.h
 * Croix Gyurek
 *
 * Will implement in stages
 */

#ifndef USER_H_EXISTS
#define USER_H_EXISTS

#include "linkedList.h"
#include "message.h"
#include "credentials.h"
#include "rsaKey.h"

class User {
    protected:
        std::string username;
        LinkedList<Message> messages;
        // public/private key go here
        RSAKey key;
        std::string maskedDKey;
        std::string maskSalt;

        // protected method
        // can't let random people re-mask my key!
        void maskKey(std::string, std::string);
    public:
        User();
        // no need for a destructor I think...?
        // constructor for a fresh account
        // (the randomizer is just to generate the salts for the password)
        User(std::string, std::string, RSAKey, Randomizer&);
        // constructor for reading an existing account
        User(std::string, RSAKey, std::string, std::string);

        // Easy getters
        std::string getUsername();
        RSAKey getPublicKey();
        std::string getMaskedDKey();
        std::string getMaskSalt();
        bool canDecrypt();

        bool checkLogin(std::string, std::string);
        bool changePassword(std::string, std::string);
        // There will be no "getMessages()" method.
        void showMessages();
        int getNumMessages();
        Message* getMessage(int);
        void deleteMessage(int);
        void readMessage(Message*);
        // sendMessage is more System than User
        void addMessage(Message*, bool);

        bool unmaskKey(std::string);
};

void testUser();

#endif

