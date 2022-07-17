/*
 * Message.cpp
 * Croix Gyurek
 *
 * Abstraction
 */

#include "message.h"

Message::Message() {
    from = "";
    date = "";
    storedSubject = "";
    plainSubject = "";
    storedBody = "";
    plainBody = "";
    encrypted = false;
    decrypted = true;
    unread = true;
};
// I like to preface constructor parameters with "given"
// you really tempted me with a using name-space std here
Message::Message(int givenID,            \
                 std::string givenSender, \
                 std::string givenDate,    \
                 std::string givenSubject,  \
                 std::string givenBody,      \
                 bool isEncrypted,            \
                 bool isUnread) {
    id = givenID;
    from = givenSender;
    date = givenDate;
    storedSubject = givenSubject;
    storedBody = givenBody;
    encrypted = isEncrypted;
    unread = isUnread;

    // If encrypted set the plain texts to empty
    if (encrypted) {
        plainSubject = std::string("");
        plainBody = std::string("");
        decrypted = false;
    } else {
        // this message is already in plain text
        decrypted = true;
        plainSubject = givenSubject;
        plainBody = givenBody;
    }
};

int Message::getID() {
    return id;
};
std::string Message::getSender() {
    return from;
};
std::string Message::getDate() {
    return date;
};
std::string Message::getStoredSubject() {
    return storedSubject;
};
std::string Message::getStoredBody() {
    return storedBody;
};
std::string Message::getPlainSubject() {
    if (encrypted) {
        if (decrypted) {
            return plainSubject;
        } else {
            // message has not been decrypted yet
            return "[ENCRYPTED]";
        }
    } else {
        return storedSubject;
    }
};
std::string Message::getPlainBody() {
    if (encrypted) {
        if (decrypted) {
            return plainBody;
        } else {
            return "[ENCRYPTED]";
        }
    } else {
        return storedBody;
    }
};

bool Message::isUnread() {
    return unread;
};
bool Message::isEncrypted() {
    return encrypted;
};
bool Message::isDecrypted() {
    return decrypted;
};

void Message::markRead() {
    unread = false;
};
void Message::markUnread() {
    unread = true;
};

void Message::recordDecryption(std::string subj, std::string body) {
    if (encrypted) {
        decrypted = true;
        plainSubject = subj;
        plainBody = body;
    }
};

