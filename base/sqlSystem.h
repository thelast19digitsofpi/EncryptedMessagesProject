/*
 * system.h
 * Croix Gyurek
 *
 * The last leg of this long journey!
 */

#ifndef SYSTEM_H_EXISTS
#define SYSTEM_H_EXISTS

#include <string>
#include <sqlite3.h>
#include "message.h"
#include "user.h"
#include "linkedList.h"
#include "randomizer.h"

class SQLSystem {
    protected:
        sqlite3* database;
        LinkedList<User> users;
    public:
        ~SQLSystem();

        void init(std::string);
        void readUsers();
        User* userLogin(std::string, std::string);
        void loadMessages(User*);
};

#endif

