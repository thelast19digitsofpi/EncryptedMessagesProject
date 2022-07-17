/*
 * message.h
 * Croix Gyurek
 *
 **
 ***
 ****
 *****
 ******/

#ifndef MESSAGE_H_EXISTS
#define MESSAGE_H_EXISTS

#include <string>

class Message {
    protected:
        int id;
        std::string from;
        std::string date;
        std::string storedSubject;
        std::string storedBody;
        std::string plainSubject;
        std::string plainBody;
        bool unread;
        bool encrypted;
        bool decrypted;
    public:
        Message();
        Message(int, std::string, std::string, std::string, std::string, bool, bool);
        
        int getID();
        std::string getSender();
        std::string getDate();
        std::string getStoredSubject();
        std::string getStoredBody();
        // These return empty if the message has not been decrypted
        std::string getPlainSubject();
        std::string getPlainBody();
        bool isUnread();
        bool isEncrypted();
        bool isDecrypted();

        void recordDecryption(std::string, std::string);

        void markRead();
        void markUnread();
};

#endif
// 42 lines, neat
