/*
 * user.cpp
 * Croix Gyurek
 *
 * Things aren't looking so bad now...
 * (note to self: forget that I wrote the above)
 */

// printf is far superior to std::cout
// I even sometimes used System.out.printf in bankonit
#include <cstdio>

#include "user.h"

User::User() {
    username = "";
    messages = LinkedList<Message>();
};

// when constructing a fresh account
// the full key is generated and masked
User::User(std::string givenUsername, std::string password, RSAKey fullKey, Randomizer& generator) {
    username = givenUsername;
    // now create a message list
    messages = LinkedList<Message>();
    // and mask the RSA key
    key = fullKey;
    std::string salt = generator.randomHex(40);
    maskKey(password, salt);
};
// when constructing a User from the database
// we only know part of the key
User::User(std::string givenUsername, RSAKey partialKey, std::string maskedOtherPart, std::string givenMaskSalt) {
    username = givenUsername;
    key = partialKey;
    maskedDKey = maskedOtherPart;
    maskSalt = givenMaskSalt;
    messages = LinkedList<Message>();
};

// Getters
std::string User::getUsername() {
    return username;
};
// Returns a PARTIAL RSAKey object. Calling getD() will give a zero.
RSAKey User::getPublicKey() {
    // in-CAP-suh-LAY-shun!
    // they can realize this is a public-only key because getD() gives zero
    return RSAKey(key.getN(), key.getE());
};
// that extra D is going to cause pain and delight
// note: knowing all of this gives no useful information to hackers
std::string User::getMaskedDKey() {
    return maskedDKey;
};
std::string User::getMaskSalt() {
    return maskSalt;
};
bool User::canDecrypt() {
    // If the D is zero you have a partial (public only) key
    return !key.getD().isZero();
};

bool User::checkLogin(std::string usernameGuess, std::string passwordGuess) {
    // I purposely do this inefficiently so that you cannot tell by the quick answer which one is incorrect
    // The compiler may optimize this away tho
    bool usernameCorrect = (usernameGuess == username);
    bool passwordCorrect = unmaskKey(passwordGuess);
    return usernameCorrect && passwordCorrect;
};
bool User::changePassword(std::string oldPassword, std::string newPassword) {
    // If you don't have the key you are not "logged in"
    if (!canDecrypt()) {
        std::cout << "not logged in" << std::endl;
        return false;
    }

    // Check if the old password is correct
    if (checkLogin(username, oldPassword)) {
        // Change the password by re-masking the key
        // hmmm salt doesn't change...
        maskKey(newPassword, maskSalt);
        return true;
    } else {
        return false;
    }
};

void User::showMessages() {
    // Show the summary list consisting of just the subjects
    int messageNum = 1;
    Node<Message>* curNode = messages.getHead();
    // If empty, say so
    if (curNode == 0) {
        printf("You have no messages.\n");
    } else {
        // used printf so I have format strings
        printf("U = Unread.\n");
        printf("Num |   |      Date      |      Sender      | Subject\n");
        while (curNode != 0) {
            // Print the message header
            Message* msgPtr = curNode->getPayload();
            std::string plainSubject;
            if (msgPtr->isEncrypted()) {
                plainSubject = "[Encrypted]";
            } else {
                plainSubject = msgPtr->getPlainSubject();
            }
            printf("%3d | %c | %-14s | %-16s | %s\n",
                    messageNum,
                    msgPtr->isUnread() ? 'U' : ' ',
                    msgPtr->getDate().c_str(),
                    msgPtr->getSender().c_str(),
                    plainSubject.c_str() );
            messageNum += 1;
            curNode = curNode->getNext();
        }
        printf("U = Unread.\n");
    }
};
// note these are linear-time operations so use sparingly
int User::getNumMessages() {
    return messages.getSize();
};
Message* User::getMessage(int which) {
    return messages.getFromHead(which)->getPayload();
};
void User::deleteMessage(int which) {
    // this is rather simple...
    messages.remove(messages.getFromHead(which));
};

void User::addMessage(Message* msgPtr, bool atEnd) {
    Node<Message>* nodePtr = new Node<Message>(msgPtr);
    if (atEnd) {
        messages.insertTail(nodePtr);
    } else {
        messages.insertHead(nodePtr);
    }
};

void User::readMessage(Message* msgPtr) {
    if (!canDecrypt()) {
        std::cout << "You are not logged in and cannot access this message. That, or something is seriously wrong..." << std::endl;
        return;
    }
    std::string subject;
    std::string body;
    if (msgPtr->isEncrypted()) {
        // message is secret
        if (msgPtr->isDecrypted()) {
            // ...but you already decoded it
            subject = msgPtr->getPlainSubject();
            body = msgPtr->getPlainBody();
        } else {
            // Decrypt it.
            std::string encryptedSubject = msgPtr->getStoredSubject();
            std::string encryptedBody = msgPtr->getStoredBody();
            subject = key.decrypt(encryptedSubject);
            body = key.decrypt(encryptedBody);
            msgPtr->recordDecryption(subject, body);
        }
    } else {
        subject = msgPtr->getPlainSubject();
        body = msgPtr->getPlainBody();
    }
    
    std::cout << std::endl;
    std::cout << "--- MESSAGE ---" << std::endl;
    std::cout << "From: " << msgPtr->getSender() << std::endl;
    std::cout << "Date: " << msgPtr->getDate() << std::endl;
    std::cout << "Subject: " << subject << std::endl;
    std::cout << std::endl;
    std::cout << body << std::endl << std::endl;
    std::cout << "[end message]" << std::endl;
    msgPtr->markRead();
};

// Masks the user's decryption key.
// You can only recover it if you know the password.
// Protected method.
void User::maskKey(std::string password, std::string salt) {
    if (key.getD().isZero()) {
        throw std::string("Error: Cannot mask nonexistent key");
    }
    
    // get the decryption key
    std::string dHex = key.getD().convertToHex();
    // remove leading zeros
    dHex = dHex.substr( dHex.find_first_not_of('0') );
    int dHexDigits = dHex.length();
    
    maskSalt = salt;
    std::stringstream maskStream;
    for (int chunk = 0; chunk < dHexDigits; chunk += 64) {
        // For this chunk of 64 bits we generate a different hash
        // putting the string 0 or 64 or 128 etc makes it really different
        std::string chunkHash = sha256(password + maskSalt + std::to_string(chunk));
        for (int i = 0; i < 64; i++) {
            // Don't waste time encrypting characters outside dHex
            if (chunk + i < dHexDigits) {
                // use a stringstream to convert the characters into ints
                int deDigit, haDigit;
                std::stringstream converter;
                // e.g. get 10th character of hash for 64-127 and 74th of dKey
                converter << dHex[chunk + i] << ' ' << chunkHash[i];
                // and convert them into numbers
                converter >> std::hex >> deDigit;
                converter >> std::hex >> haDigit;
                // modular arithmetic!
                int encryptedDigit = (deDigit + haDigit) % 16;
                maskStream << std::hex << encryptedDigit;
            }
        }
    } // end outer for
    // we don't return anything
    maskedDKey = maskStream.str();
}; // end maskKey

// Attempts to unmask the user's decryption key.
bool User::unmaskKey(std::string password) {
    // un-encrypt the key
    std::stringstream decryption;
    int numDigits = maskedDKey.length();
    // we have password, maskSalt, and maskedDKey.
    for (int chunk = 0; chunk < numDigits; chunk += 64) {
        // same hash method as maskKey()
        std::string chunkHash = sha256(password + maskSalt + std::to_string(chunk));
        for (int i = 0; i < 64; i++) {
            if (chunk + i < numDigits) {
                // convert the hex characters to ints
                std::stringstream converter;
                converter << maskedDKey[chunk + i] << ' ' << chunkHash[i];
                int maskDigit;
                converter >> std::hex >> maskDigit;
                int hashDigit;
                converter >> std::hex >> hashDigit;
                // modular arithmetic!
                int decryptedDigit = (16 + maskDigit - hashDigit) % 16;
                decryption << std::hex << decryptedDigit;
            }
        }
    }
    // we have a key but it may be the wrong key
    BigNumber probableKey(decryption.str());

    // test the key to make sure it works
    // hey, funny that a 42 will make it into finished code!
    BigNumber testMessage(42);
    // encrypt the 42
    BigNumber cipherTest = BigNumber::modExp(testMessage, key.getE(), key.getN());
    // if this works this should give 42
    BigNumber result = BigNumber::modExp(cipherTest, probableKey, key.getN());
    if (result == testMessage) {
        // record this key
        if (key.getD().isZero()) {
            key.recordD(probableKey);
        }
        return true;
    } else {
        // Wrong password.
        return false;
    }
};

void testUser() {
    // "dead bits" is the salt; I do not endorse this password
    Randomizer r1("297388273ehhe489273");
    RSAKey k1(1000, r1);
    User u1("fprefect42", "password1!", k1, r1);
    
    std::cout << "Authenticating wrong password" << std::endl;
    bool shouldFail = u1.unmaskKey("wrong_password0");
    std::cout << "Authenticating correct password" << std::endl;
    // <--- good breakpoint for gdb
    bool shouldWork = u1.unmaskKey("password1!");
    std::cout << std::boolalpha << shouldFail << ' ' << shouldWork << std::endl;


    // add some messages
    Message* m1 = new Message(1, "arthurdent", "01/42/98 00:00", "Hi", "How are things going Ford?", false, true);
    Message* m2 = new Message(2, "beeblebrox", "05/05/94 00:00", "Important message", "[whatever sort of thing Zaphod would send Ford]", false, true);
    Message* m3 = new Message(3, "SYSTEM", "01/01/70 00:00", "Guide Update", "The Hitchhiker's Guide has just been updated. Earth is now \"mostly\" harmless.", false, true);
    
    m1->markRead();
    u1.showMessages();
    u1.addMessage(m2, false);
    u1.addMessage(m1, false);
    u1.addMessage(m3, true);
    u1.showMessages();
    Message* mp = u1.getMessage(2);
    u1.readMessage(mp);
    u1.showMessages();

    // change password
    printf("\nChanging Password...\n");
    printf("Wrong password:\n");
    // should be password1! not password! (the 1 is missing)
    bool s1 = u1.changePassword("password!", "bad_password");
    // confirm the password is still "password1!"
    bool s1test = u1.checkLogin("fprefect42", "password1!");
    printf("%c %c (expect n Y)\n", s1 ? 'Y' : 'n', s1test ? 'Y' : 'n');
    // change it correctly
    // this is not actually a good password
    bool s2 = u1.changePassword("password1!", "good_password");
    bool s2test = u1.checkLogin("fprefect42", "good_password");
    bool oldIsGone = u1.checkLogin("fprefect42", "password1!");
    printf("Correct password:\n");
    printf("%c %c %c (expect Y Y n)\n",
            s2 ? 'Y' : 'n',
            s2test ? 'Y' : 'n',
            oldIsGone ? 'Y' : 'n');
};


