/*
 * fileSystem.h
 * Croix Gyurek
 *
 * Did I not already make such a header?
 */

#ifndef FILESYSTEM_H_EXISTS
#define FILESYSTEM_H_EXISTS

#include <string>
#include "user.h"
#include "message.h"
#include "linkedList.h"

class FileSystem {
    protected:
        LinkedList<User> users;
        std::string databaseFile;
        
        // this is powerful so we hide it
        User* getUserByName(std::string);
    public:
        FileSystem();
        ~FileSystem();

        void init(std::string);
        void save();
        User* login(std::string, std::string);
        bool isUsernameValid(std::string);
        bool isUsernameFree(std::string);
        void addNewUser(User*);

        void viewMessages(User*);
        void composeMessageMenu(User*);
        void composeMessageMenu(User*, User*, std::string);
        void changePasswordMenu(User*);

        // I couldn't resist
        // (altho the array is useless w/o a length)
        static void main(std::string args[]);

        static void test();
};

#endif

