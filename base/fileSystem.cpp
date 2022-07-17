/*
 * fileSystem.cpp
 * Croix Gyurek
 *
 * This project using files.
 * Advantages:
 * - Easier to write the code
 * Disadvantages:
 * - Only one person can use it at a time or else reallyBadThingsHappen()
 */

#include "fileSystem.h"
#include "randomizer.h"
#include "mydefs.h"
#include <iostream>
#include <fstream>
#include <cctype>
#include <ctime>

// these may not be really that necessary but...
FileSystem::FileSystem() {
    databaseFile = ""; // ok at least make sure it gets initialized
};
FileSystem::~FileSystem() {
    // probably nothing? who knows
};

/*
File format:

BEGIN USER: <username>
<N>|<E>|<maskedD>|<maskSalt>
Begin Message
>>> <ID>|<encrypted=1,not=0>|<unread:U/R><date>|<sender>
>>> <subject>
>>> <eachLineOfTheBody>
>>> <moreLinesMaybe>
End Message
Begin Message
[...]
End Message
END USER
BEGIN USER: <username2>
[...]
*/

void FileSystem::init(std::string fileName) {
    std::ifstream dataFile;
    dataFile.open(fileName);
    if (dataFile.is_open()) {
        // do the hard work
        bool keepGoing1 = true;
        while (keepGoing1) {
            std::string line1;
            getline(dataFile, line1);
            // most likely this will be "BEGIN USER: username"
            if (line1.substr(0, 10) == "BEGIN USER") {
                std::string username = line1.substr(12, 9999);
                // Get the next line
                std::string keyData;
                getline(dataFile, keyData);
                // Parse out four items from the line
                std::string nText, eText, maskedD, maskSalt;
                std::stringstream keyStream;
                keyStream << keyData;
                // first up is the public exponent N; then E, then D, then salt
                getline(keyStream, nText, '|');
                getline(keyStream, eText, '|');
                getline(keyStream, maskedD, '|');
                getline(keyStream, maskSalt); // last thing in the line, no |
                // Construct the key
                BigNumber keyN(nText);
                BigNumber keyE(eText); // almost universally 10001, maybe 10003
                RSAKey publicKey(keyN, keyE);
                // we have all we need!
                User* newUser = new User(username, publicKey, maskedD, maskSalt);

                // now read in this user's messages!
                bool keepGoing2 = true;
                while (keepGoing2) {
                    // 5 layers deep already and it will get worse
                    std::string line2;
                    getline(dataFile, line2);
                    if (line2 == "Begin Message") {
                        // start of a message
                        std::string header;
                        std::stringstream headerStream;
                        // read the header info
                        getline(dataFile, header);
                        headerStream << header;
                        std::string throwaway;
                        std::string idText, isEncryptedText, isUnreadText, date, sender;
                        // >>> can be discarded
                        getline(headerStream, throwaway, ' ');
                        // legitimate data
                        getline(headerStream, idText, '|');
                        getline(headerStream, isEncryptedText, '|');
                        getline(headerStream, isUnreadText, '|');
                        getline(headerStream, date, '|');
                        getline(headerStream, sender);
                        // conversions
                        int id = std::stoi(idText);
                        bool isEncrypted = (isEncryptedText[0] == '1'); // easier than trying to convert to an int
                        bool isUnread = (isUnreadText[0] == 'U'); // we use U for unread so we can tell the difference
                        // now get the subject
                        std::string subject;
                        getline(dataFile, subject);
                        subject = subject.substr(4, 9999);
                        // now the tricky part: the message body
                        std::stringstream bodyStream;
                        bool keepGoing3 = true;
                        while (keepGoing3) {
                            // Get another line
                            std::string line3;
                            getline(dataFile, line3);
                            if (line3.substr(0, 3) == ">>>") {
                                // 8 layers deep!!!
                                // a continuing line
                                bodyStream << line3.substr(4, 9999);
                                bodyStream << std::endl;
                            } else {
                                // Not a continuation of the message.
                                // Assume we are ending the message.
                                // todo: perhaps detect "END USER"?
                                keepGoing3 = false;
                            }
                        } // end keepGoing3 loop
                        std::string body = bodyStream.str();
                        if (body.size() > 0) {
                            body.pop_back(); // deletes the trailing newline
                        }
                        // construct the message!
                        Message* msgPtr = new Message(id, sender, date, subject, body, isEncrypted, isUnread);
                        newUser->addMessage(msgPtr, true); // put the message at the end, so most recent message is last
                    } else if (line2 == "END USER") {
                        // The user may have no messages
                        keepGoing2 = false; // loop 2 was for the user
                    } else {
                        if (dataFile.eof()) {
                            // we are totally done
                            keepGoing2 = false;
                            keepGoing1 = false;
                        } else {
                            // something else?!
                            std::cerr << "Unexpected line detected!" << std::endl;
                            std::cerr << line2 << std::endl;
                        }
                    } // end line2 if statement
                } // end keepGoing2 loop
                // the user is ready, add them to the list
                Node<User>* userNode = new Node<User>(newUser);
                users.insertTail(userNode);
            } else {
                // we hit a line between 2 user definitions
                // ignore it or perhaps print something out?
            }
            // if we hit EOF then stop
            if (dataFile.eof()) {
                keepGoing1 = false;
            }
        } // end while keepGoing1
    } else { // if file is not found
        std::cout << "You could be the first user!" << std::endl;
    }

    // WHEW!!
    this->databaseFile = fileName;
    std::cout << "All systems are go." << std::endl;
    dataFile.close();
};


/*
File format:

BEGIN USER: <username>
<N>|<E>|<maskedD>|<maskSalt>
Begin Message
>>> <ID>|<encrypted=1,not=0>|<U/R>|<date>|<sender>
>>> <subject>
>>> <eachLineOfTheBody>
>>> <moreLinesMaybe>
End Message
Begin Message
[...]
End Message
END USER
BEGIN USER: <username2>
[...]
*/

void FileSystem::save() {
    std::ofstream outputFile;
    outputFile.open(databaseFile);
    // linked lists loop like... can't think of a 5th L word
    for (Node<User>* curNode = users.getHead(); // what's the sentry...
            curNode != nullptr; // how does it end...
            curNode = curNode->getNext() // how does it change?
    ) {
        User* userPtr = curNode->getPayload();
        outputFile << "BEGIN USER: " << userPtr->getUsername() << std::endl;
        RSAKey publicKey = userPtr->getPublicKey();
        BigNumber N = publicKey.getN();
        BigNumber E = publicKey.getE();
        outputFile << N.convertToHex() << '|'
                   << E.convertToHex() << '|'
                   << userPtr->getMaskedDKey() << '|'
                   << userPtr->getMaskSalt() << std::endl;
        // now write all messages
        // annoyingly due to my encapsulation skills this is O(n^2)
        // we don't have access to the message list
        int numMessages = userPtr->getNumMessages();
        for (int i = 0; i < numMessages; i++) {
            outputFile << "Begin Message" << std::endl;
            Message* msgPtr = userPtr->getMessage(i);
            int id = msgPtr->getID();
            // one of these days I will want to write using namespace std
            // and then I will realize that `using std::string` is safer
            std::string sender = msgPtr->getSender();
            std::string date = msgPtr->getDate();
            std::string subject = msgPtr->getStoredSubject();
            std::string body = msgPtr->getStoredBody();
            bool isEncrypted = msgPtr->isEncrypted();
            bool isUnread = msgPtr->isUnread();
            std::string lineStarter = ">>> "; // trust me it looks bad in the middle of those <<'s
            // put the header information in
            outputFile << lineStarter
                << id << '|'
                << (isEncrypted ? 1 : 0) << '|'
                << (isUnread ? 'U' : 'R') << '|'
                << date << '|'
                << sender << std::endl;
            // now the subject and the body
            outputFile << lineStarter << subject << std::endl;
            // the body is tricky (it may have several lines)
            std::stringstream originalBodyStream;
            originalBodyStream << body;
            bool keepGoing = true;
            while (keepGoing) {
                std::string line;
                getline(originalBodyStream, line);
                outputFile << lineStarter << line << std::endl;
                if (originalBodyStream.eof()) {
                    keepGoing = false;
                }
            }
            outputFile << "End Message" << std::endl;
        } // end for loop of messages
        outputFile << "END USER" << std::endl;
    } // end for loop of users

    // it's all over now, close the file!
    outputFile.close();
};

// Returns null if no user is found
User* FileSystem::login(std::string usernameGuess, std::string passwordGuess) {
    for (Node<User>* curNode = users.getHead();
            curNode != nullptr;
            curNode = curNode->getNext()) {
        User* userPtr = curNode->getPayload();
        if (userPtr->checkLogin(usernameGuess, passwordGuess)) {
            // login successful
            return userPtr;
        }
    }
    // incorrect credentials
    return nullptr;
};

bool FileSystem::isUsernameValid(std::string name) {
    int len = name.length();
    if (len < 2 || len > 15) {
        return false;
    }
    for (int i = 0; i < len; i++) {
        char c = name[i];
        if (c == ' ' || c == '\n') {
            return false; // no spaces or new lines allowed
        }
    }
    // reserved name
    if (name == "SYSTEM") {
        return false;
    }
    return true; // no errors, no problem
};

bool FileSystem::isUsernameFree(std::string name) {
    for (Node<User>* curNode = users.getHead();
            curNode != nullptr;
            curNode = curNode->getNext()) {
        User* curUser = curNode->getPayload();
        if (curUser->getUsername() == name) {
            return false; // found a match
        }
    }
    return true;
};

// private method
User* FileSystem::getUserByName(std::string name) {
    for (Node<User>* curNode = users.getHead();
            curNode != nullptr;
            curNode = curNode->getNext()) {
        User* curUser = curNode->getPayload();
        if (curUser->getUsername() == name) {
            return curUser;
        }
    }
    return nullptr;
};

void FileSystem::addNewUser(User* userPtr) {
    if (userPtr != nullptr) {
        Node<User>* newNode = new Node<User>(userPtr);
        users.insertHead(newNode);
    }
};

// this is not a class method
int getNumberFromUser() {
    std::string userText;
    getline(std::cin, userText);
    try {
        int num = std::stoi(userText);
        return num;
    } catch (...) {
        // return -1 in case of any error
        return -1;
    }
};

bool isValidPassword(std::string password) {
    int length = password.length();
    if (length < 8 || length > 20) {
        return false;
    }

    bool hasLetter = false, hasDigit = false, hasSpecial = false;
    for (int i = 0; i < length; i++) {
        char c = password[i];
        if (isalpha(c)) {
            hasLetter = true;
        } else if (isdigit(c)) {
            hasDigit = true;
        } else if (ispunct(c)) {
            hasSpecial = true;
        }
        // anything else is spaces or worse, don't count them
    }
    return hasLetter && hasDigit && hasSpecial;
};

// this one is more complicated than may initially seem
void FileSystem::viewMessages(User* userPtr) {
    bool keepGoing = true;
    while (keepGoing) {
        userPtr->showMessages();
        int numMessages = userPtr->getNumMessages();
        if (numMessages == 0) {
            std::cout << "Returning to menu." << std::endl;
            keepGoing = false;
        } else {
            std::cout << std::endl
                << "Which message to read?" << std::endl
                << "Enter number (1-" << numMessages << ", 0 to leave)" << std::endl;
            int whichMsg = getNumberFromUser();
            // Is this encapsulation?
            if (whichMsg >= 1 && whichMsg <= numMessages) {
                int messageIndex = whichMsg - 1; // classic OBOE
                // Read that message.
                Message* msgPtr = userPtr->getMessage(messageIndex);
                std::cout << "may take a few seconds..." << std::endl;
                userPtr->readMessage(msgPtr);
                std::cout << std::endl << " << Press Enter To Continue >> " << std::endl;
                std::string waste;
                getline(std::cin, waste);

                bool keepGoing2 = true;
                while (keepGoing2) {
                    std::cout << "You may either:" << std::endl
                        << "  (1) Read the message again" << std::endl
                        << "  (2) Mark the message as UNREAD (and go back)" << std::endl
                        << "  (3) Delete the message" << std::endl
                        << "  (4) Reply (note: message body is not included)" << std::endl
                        << "  (0) Go back (to the message list)" << std::endl;
                    int messageChoice = getNumberFromUser();
                    if (messageChoice == 1) {
                        // useful if you get caught in a bunch of failed options and forget what the message was
                        userPtr->readMessage(msgPtr);
                        std::cout << " << Press Enter To Continue >> " << std::endl;
                        getline(std::cin, waste);
                    } else if (messageChoice == 2) {
                        // mark message as unread and then leave
                        msgPtr->markUnread();
                        keepGoing2 = false;
                    } else if (messageChoice == 3) {
                        // delete
                        std::cout << "Are you sure you wish to DELETE this message?" << std::endl;
                        std::cout << "Warning: This action cannot be undone." << std::endl;
                        std::cout << "(you might want to make sure you are deleting the correct message...)" << std::endl;
                        std::cout << "Enter Y or y (or anything starting with a Y) to confirm: ";
                        std::string confirmDelete;
                        getline(std::cin, confirmDelete);
                        if (confirmDelete[0] == 'y' || confirmDelete[0] == 'Y') {
                            userPtr->deleteMessage(messageIndex);
                            keepGoing2 = false; // we have to exit because the message is (probably literally) deleted
                        } else {
                            std::cout << "That doesn't look like a Y, so canceling..." << std::endl;
                        }
                    } else if (messageChoice == 4) {
                        // the only tricky thing here is getting the new subject
                        std::string oldSubject = msgPtr->getPlainSubject();
                        std::string newSubject;
                        int oldSubjectLength = oldSubject.length();
                        if (oldSubjectLength > SUBJECT_CAP - 4) {
                            // subject is too long
                            newSubject = "Re: " + oldSubject.substr(0, SUBJECT_CAP - 7) + "...";
                        } else {
                            // it fits with a "Re: " on it
                            newSubject = "Re: " + oldSubject;
                        }
                        std::string senderUsername = msgPtr->getSender();
                        User* senderPtr = getUserByName(senderUsername);
                        if (senderPtr != nullptr) {
                            // take them to the compose message menu
                            // hey look, polymorphism
                            composeMessageMenu(userPtr, senderPtr, newSubject);
                            keepGoing2 = false;
                        } else {
                            // no user by that name exists
                            std::cout << "You cannot reply to this message." << std::endl;
                            std::cout << "Probably the sender no longer exists OR it is a system message." << std::endl;
                        }
                    } else if (messageChoice == 0) {
                        std::cout << "Exiting." << std::endl;
                        keepGoing2 = false; // they just exit
                    } else {
                        std::cout << "Invalid choice, please try again." << std::endl;
                    } // end messageChoice if chain
                } // end keepGoing2
            } else if (whichMsg == 0) {
                keepGoing = false;
            } else {
                std::cout << "Invalid entry." << std::endl;
            } // end whichMsg chain
        } // end if 0 messages chain
        std::cout << "\nPress Enter To Continue...\n" << std::endl;
        std::string waste;
        getline(std::cin, waste);
    } // end outer while
};

// has 2 forms; one calls the second
void FileSystem::composeMessageMenu(User* userPtr) {
    // we need to get the subject line and the recipient
    std::cout << "Message Writer" << std::endl;
    // print out all users
    std::cout << "To whom do you wish to send this message?" << std::endl;
    int num = 0;
    for (Node<User>* curNode = users.getHead();
            curNode != nullptr;
            curNode = curNode->getNext()) {
        num += 1;
        std::cout << " (" << num << ") " << curNode->getPayload()->getUsername() << std::endl;
    }
    // num is now the length of the list
    std::cout << "Enter the number (1-" << num << ") or 0 to cancel:" << std::endl;
    int whichUser = getNumberFromUser();
    if (whichUser >= 1 && whichUser <= num) {
        // this subtract 1 thing is almost second nature
        // but also easy to screw up
        User* otherUser = users.getFromHead(whichUser - 1)->getPayload();
        // now you need a subject
        std::cout << "Enter the subject of the message (or nothing to cancel)." << std::endl;
        std::cout << "Subject: ";
        std::string subject;
        getline(std::cin, subject);
        // now do the hard work in the other function
        composeMessageMenu(userPtr, otherUser, subject);
    } else if (whichUser == 0) {
        std::cout << "Canceling." << std::endl;
    } else {
        std::cout << "Invalid option. Canceling." << std::endl;
    }
    std::cout << std::endl << "PRESS ENTER TO CONTINUE" << std::endl << std::endl;
    std::string waste;
    getline(std::cin, waste);
};

#define MAX_LINES 9
void FileSystem::composeMessageMenu(User* userPtr, User* toWhom, std::string subject) {
    // linked lists are cooooool
    std::cout << "\n\n\n\n\n" << std::endl;
    std::cout << "===== MESSAGE BUILDER =====" << std::endl;
    std::cout << "Note: Messages are limited to " << MAX_LINES << " lines and " << MESSAGE_CAP << " characters." << std::endl;
    std::cout << "Messages with multiple lines must be built one line at a time." << std::endl;
    std::cout << std::endl;
    
    // what I said.
    LinkedList<std::string> lineList;
    bool keepGoing = true;
    while (keepGoing) {
        // some spacing
        std::cout << "\n\n\n\n\n" << std::endl;
        // Display message content.
        int lineNumber = 0;
        int numChars = 0;
        for (Node<std::string>* lineNode = lineList.getHead();
                lineNode != nullptr;
                lineNode = lineNode->getNext()) {
            // Print this line
            lineNumber += 1;
            std::string* line = lineNode->getPayload();
            std::cout << "[" << lineNumber << "] >>> " << *line << std::endl;
            numChars += line->length() + 1;
        }
        int numLines = lineNumber; // better name
        std::cout << "lines: " << numLines << '/' << MAX_LINES << ' '
            << "characters: " << numChars << '/' << MESSAGE_CAP << std::endl;
        std::cout << std::endl;
        // the menu
        std::cout << "Your options: " << std::endl
            << "(1) Change an existing line" << std::endl
            << "(2) Add a new line" << std::endl
            << "(3) Insert a new line" << std::endl
            << "(4) Delete a line" << std::endl
            << "(5) Send the message" << std::endl
            << "(0) Cancel and discard the message" << std::endl;
        std::cout << "Enter choice: ";
        int choice = getNumberFromUser();
        if (choice == 1) {
            // change line
            std::cout << "Which line (1-" << numLines << ", 0 to cancel)? ";
            int whichLine = getNumberFromUser();
            if (whichLine >= 1 && whichLine <= numLines) {
                // Get that line from the linked list
                Node<std::string>* lineNode = lineList.getFromHead(whichLine - 1);
                // change it
                std::cout << "Change the line to... (enter nothing to cancel)" << std::endl;
                std::string replacement;
                getline(std::cin, replacement);
                std::cout << "Your line now reads: " << std::endl
                    << replacement << std::endl
                    << "Do you wish to use this? 1 = Yes: ";
                int confirm = getNumberFromUser();
                if (confirm == 1) {
                    // payloads have to be on the heap
                    lineNode->setPayload(new std::string(replacement));
                } else {
                    std::cout << "Canceling" << std::endl;
                }
            } else if (whichLine == 0) {
                std::cout << "Canceling" << std::endl;
            } else {
                std::cout << "Invalid line. Canceling." << std::endl;
            }
        } else if (choice == 2) {
            // add a line
            if (numLines < MAX_LINES) {
                std::cout << "Enter the text to add: " << std::endl;
                std::string newLine;
                getline(std::cin, newLine);
                std::string* newLineHeap = new std::string(newLine);
                // create a node
                auto lineNode = new Node<std::string>(newLineHeap);
                lineList.insertTail(lineNode);
                std::cout << "Added successfully!" << std::endl;
            } else {
                std::cout << "You have maxed out the lines." << std::endl
                    << "You need to delete or change an existing one." << std::endl;
            }
        } else if (choice == 3) {
            // insert a line
            if (numLines < MAX_LINES) {
                std::cout << "Enter the text to insert: " << std::endl;
                std::string newLine;
                getline(std::cin, newLine);
                std::string* newLineHeap = new std::string(newLine);
                // create a node, again
                auto lineNode = new Node<std::string>(newLineHeap);
                // now we need to find where to insert.
                std::cout << "Where to insert this line?" << std::endl
                    << "  Enter 1-" << numLines << " to insert BEFORE that line" << std::endl
                    << "  Enter " << (numLines + 1) << " to insert at the end" << std::endl
                    << "  Enter 0 to cancel" << std::endl;
                std::cout << "> ";
                int insertPos = getNumberFromUser();
                if (insertPos >= 1 && insertPos <= numLines) {
                    auto placeNode = lineList.getFromHead(insertPos - 1);
                    lineList.insertBefore(lineNode, placeNode);
                    std::cout << "Inserted successfully!" << std::endl;
                } else if (insertPos > numLines) {
                    // add to the end
                    lineList.insertTail(lineNode);
                    std::cout << "Success" << std::endl;
                } else if (insertPos == 0) {
                    std::cout << "Canceling." << std::endl;
                    delete lineNode; // which will delete the payload
                } else {
                    std::cout << "Invalid number. Canceling." << std::endl;
                    delete lineNode; // ditto above
                }
            } else {
                std::cout << "You have maxed out the lines." << std::endl
                    << "You can only delete or change existing lines, or send the message as is." << std::endl;
            }
        } else if (choice == 4) {
            std::cout << "Which line to delete (1-" << numLines << ", 0 = cancel)? ";
            int delPos = getNumberFromUser();
            if (delPos >= 1 && delPos <= numLines) {
                auto lineNode = lineList.getFromHead(delPos - 1);
                std::cout << "Confirm that you want to delete the following line?" << std::endl
                    << ">>> " << *(lineNode->getPayload()) << std::endl
                    << "Enter 1 to confirm: ";
                int confirmDelete = getNumberFromUser();
                if (confirmDelete == 1) {
                    lineList.remove(lineNode);
                    std::cout << "Deleting!" << std::endl;
                } else {
                    std::cout << "Canceling delete. Line is still there." << std::endl;
                }
            } else if (delPos == 0) {
                std::cout << "OK, nothing will be deleted." << std::endl;
            } else {
                std::cout << "Invalid number, canceling." << std::endl;
            }
        } else if (choice == 5) {
            // the most important one of all!
            // (well, maybe 2 and 3 are important too)
            if (numChars <= MESSAGE_CAP) {
                // complicated C junk to get the current time
                // thanks stack overflow question #16357999
                time_t rawtime;
                struct tm * timeInfo;
                time(&rawtime);
                timeInfo = localtime(&rawtime);

                char dateTime[20];
                strftime(dateTime, 20, "%m/%d/%y %H:%M", timeInfo);
                std::cout << "Dated " << dateTime << std::endl;
                std::string dateString(dateTime);
                // build the message!
                std::stringstream messageStream;
                for (Node<std::string>* curNode = lineList.getHead();
                        curNode != nullptr;
                        curNode = curNode->getNext()) {
                    messageStream << *(curNode->getPayload()) << std::endl;
                }
                std::string messageBody = messageStream.str();
                if (!messageBody.empty()) {
                    messageBody.pop_back(); // remove trailing endl
                }
                // the other user exists btw
                std::cout << "Encrypting (this could take a few seconds)" << std::endl;
                RSAKey key = toWhom->getPublicKey();
                std::string encryptedSubject = key.encrypt(subject);
                std::string encryptedBody = key.encrypt(messageBody);
                // ID (was made for sql), sender, date, subject, body, is unread, is encrypted
                Message* newMessage = new Message(0, userPtr->getUsername(), dateString, encryptedSubject, encryptedBody, true, true);
                toWhom->addMessage(newMessage, true); // the boolean is which side of the list.
                std::cout << "Sent Successfully!" << std::endl;
                keepGoing = false; // almost forgot to exit the loop!
            } else {
                std::cout << "Your message is too long. You need to delete a line or change one before you can send it." << std::endl;
            }
        } else if (choice == 0) {
            std::cout << "Are you sure you want to throw the ENTIRE message away?" << std::endl;
            std::cout << "Enter 1 to confirm: ";
            int confirmDeleteMessage = getNumberFromUser();
            if (confirmDeleteMessage == 1) {
                std::cout << "Your draft is deleted." << std::endl;
                keepGoing = false;
            }
        } else {
            std::cout << "Invalid choice. Try again." << std::endl;
        }

        std::cout << std::endl << " << Press Enter To Continue >> " << std::endl << std::endl;
        std::string ignore;
        getline(std::cin, ignore);
    }
    // wow...
};

void FileSystem::changePasswordMenu(User* userPtr) {
    std::string oldPassword, newPassword1, newPassword2;
    std::cout << "Please enter your current password: ";
    getline(std::cin, oldPassword);
    // check login
    std::cout << "Passwords must have:" << std::endl
        << "  8-20 characters" << std::endl
        << "  at least one letter" << std::endl
        << "  at least one digit" << std::endl
        << "  at least one special character" << std::endl;
    std::cout << "Please enter your new password: ";
    getline(std::cin, newPassword1);
    std::cout << "Confirm your new password: ";
    getline(std::cin, newPassword2);

    if (newPassword1 == newPassword2) {
        if (isValidPassword(newPassword1)) {
            // the real check
            std::cout << "Verifying password (this may take some time)" << std::endl;
            bool success = userPtr->changePassword(oldPassword, newPassword1);
            if (success) {
                std::cout << "Password changed successfully!" << std::endl;
            } else {
                std::cout << "HA! Gotcha! Your password is incorrect! Did I fool you?" << std::endl;
            }
        } else {
            std::cout << "Your new password is not valid. Remember the rules I specified?" << std::endl;
        }
    } else {
        std::cout << "Passwords Dont Match" << std::endl;
    }
};


// This is it!
void FileSystem::main(std::string args[]) {
    FileSystem sys;
    sys.init("userdata.txt");
    
    // Null constructor won't generate any digits
    Randomizer generator;

    // Home Menu
    User* you = nullptr;
    bool keepGoing = true;
    std::cout << "Welcome to the Secret Messaging System version 0.240!" << std::endl;
    while (keepGoing) {
        std::cout << "Would you like to:" << std::endl;
        std::cout << "  (1) Log in" << std::endl
                  << "  (2) Create a new account" << std::endl
                  << "  (0) Exit" << std::endl;
        std::cout << "Choice: " << std::endl;
        int mainChoice = getNumberFromUser();
        if (mainChoice == 1) {
            // Log in to existing account
            std::string username;
            std::string password;
            std::cout << "Enter your USERNAME: ";
            getline(std::cin, username);
            std::cout << "Enter your PASSWORD: ";
            getline(std::cin, password);
            // attempt to log in
            you = sys.login(username, password);
            if (you != nullptr) {
                keepGoing = false;
            } else {
                std::cout << "Incorrect username or password." << std::endl
                    << "Please try again or create a new account." << std::endl;
            }
        } else if (mainChoice == 2) {
            std::cout << "Welcome new user!" << std::endl;
            std::cout << "Create your new account here." << std::endl;
            std::cout << "Usernames must be 2-15 characters with no spaces." << std::endl;
            std::cout << "Enter username (or nothing to cancel): ";
            std::string username;
            getline(std::cin, username);
            bool valid = true;
            bool canceled = false;
        
            // check for validity
            if (username == "") {
                canceled = true;
                valid = false;
            } else if (!sys.isUsernameValid(username)) {
                std::cout << "Invalid username! Usernames must be 2-15 characters and have no spaces." << std::endl;
                valid = false;
            } else if (!sys.isUsernameFree(username)) {
                std::cout << "That username is already taken! Please choose another." << std::endl;
                std::cout << "Or did you mean to log in?" << std::endl;
                valid = false;
            } else {
                valid = true;
            }
            if (valid) {
                // you passed the first test
                // now you need a password
                std::cout << "Enter a password. Passwords must have:" << std::endl
                    << "- 8-20 characters" << std::endl
                    << "- at least one letter" << std::endl
                    << "- at least one digit" << std::endl
                    << "- at least one special character" << std::endl;
                std::cout << "WARNING: DO **NOT** USE YOUR IU PASSWORD!!!" << std::endl;
                std::cout << "Password: ";
                // I wish there was a cross-platform way to hide the password
                std::string password;
                getline(std::cin, password);
                if (isValidPassword(password)) {
                    // confirm password
                    std::cout << "Confirm password: ";
                    std::string confirmPassword;
                    getline(std::cin, confirmPassword);
                    if (confirmPassword == password) {
                        // now they need a reliable source of randomness
                        // not the most secure thing ever but a decent try
                        std::stringstream ss;
                        bool needMoreInput = true;
                        // generator is a Randomizer
                        std::cout << "Now, just press random keys for at least 100 characters. Try not to use repetitive patterns." << std::endl;
                        while (needMoreInput) {
                            std::string line;
                            getline(std::cin, line);
                            ss << line << '\n';
                            generator.insertTime(); // good source of a small amount of entropy
                            if (ss.str().length() > 100) {
                                // you are done
                                needMoreInput = false;
                            } else {
                                std::cout << "More! More!" << std::endl;
                            }
                        }
                        // put the random junk into the generator
                        generator.giveExtraStuff(ss.str());
                        // we have our randomizer ready; get an RSA key!
                        std::cout << "Generating your encryption key... (this may take a few moments...)" << std::endl;
                        RSAKey key(1000, generator);
                        std::cout << "Done." << std::endl;
                        // construct the user!!!
                        you = new User(username, password, key, generator);
                        std::cout << "You are all set! Logging in..." << std::endl;
                        sys.addNewUser(you);
                        keepGoing = false;
                    } else { // for confirmPassword == password
                        std::cout << "Your passwords do not match." << std::endl;
                        std::cout << "Returning to menu." << std::endl;
                    }
                } else { // for password validity
                    std::cout << "Your password is not valid." << std::endl;
                    std::cout << "Returning to menu." << std::endl;
                }
            } else if (canceled) {
                std::cout << "OK, canceling." << std::endl;
            } else {
                std::cout << "Returning to menu..." << std::endl;
            }
        } else if (mainChoice == 0) {
            keepGoing = false;
            return; // no need even to update the database if you did nothing
        }
    } // end while loop
    
    // assertion.
    if (you == nullptr) {
        throw std::string("Uh Oh This Is Bad: you == nullptr");
    }

    // time for the user menu!
    keepGoing = true;
    while (keepGoing) {
        // this is rather easy because we have functions.
        std::cout << "===== MAIN MENU =====" << std::endl
            << "Welcome " << you->getUsername() << "!" << std::endl
            << "Please choose from the following: " << std::endl
            << " (1) View your messages" << std::endl
            << " (2) Send a message" << std::endl
            << " (3) Change your password" << std::endl
            << " (0) Log out" << std::endl;
        std::cout << "Choice: " << std::endl;
        int mainChoice = getNumberFromUser();
        if (mainChoice == 1) {
            sys.viewMessages(you);
        } else if (mainChoice == 2) {
            sys.composeMessageMenu(you);
        } else if (mainChoice == 3) {
            sys.changePasswordMenu(you);
        } else if (mainChoice == 0) {
            keepGoing = false;
        }
    }
    
    std::cout << "Good bye, have a nice day, and all that stuff." << std::endl;
    std::cout << "Oh yeah, and remember to wash your hands and stay 2 meters away!" << std::endl;
    // well that looked esay
    sys.save();
};


void FileSystem::test() {
    // absolutely basic stuff
    std::cout << "Testing FileSystem" << std::endl;
    FileSystem s;
    
    std::cout << "Username tests" << std::endl;
    std::cout << s.isUsernameValid("reallylongbadname123456") << " (should be 0)" << std::endl;
    std::cout << "Password tests (0 0 0 0 0 1)" << std::endl;
    std::cout << isValidPassword("2short!") << ' '
        << isValidPassword("thisIsReallyLong!NotValid!!123456789") << ' '
        << isValidPassword("noDigits=Bad") << ' '
        << isValidPassword("noSpecialChars123") << ' '
        << isValidPassword("3.1415926535") << ' '
        << isValidPassword("All3RulesMet=Good") << std::endl;

    std::cout << "Enter a file name that DOES NOT ALREADY EXIST:" << std::endl;
    std::string fileName;
    getline(std::cin, fileName);
    s.init(fileName);
    std::cout << "Making test cases... 0/2" << std::endl;
    Randomizer gen("y8923283ch8923rewrF");
    RSAKey key1(950, gen);
    User* u1 = new User("user01", "password-1", key1, gen);
    std::cout << "1/2" << std::endl;
    gen.insertTime();
    RSAKey key2(950, gen);
    User* u2 = new User("user02", "password-2", key2, gen);
    std::cout << "2/2" << std::endl;

    s.addNewUser(u1);
    s.addNewUser(u2);

    std::cout << "||| Login Test |||" << std::endl;
    std::cout << "User 1 is " << (u1) << std::endl;
    std::cout << "User 2 is " << (u2) << std::endl;
    
    // try to login with bad credentials
    User* badCreds = s.login("user01", "password-2"); // they dont match.
    std::cout << "Bad credentials returned " << badCreds << " (expect 0)" << std::endl;
    // and good credentials
    User* goodCreds = s.login("user02", "password-2"); // they do match.
    std::cout << "Good credentials returned " << goodCreds << " (expect user 2)" << std::endl;

    // now try to save and load
    std::cout << "Saving..." << std::endl;
    s.save();

    // note: please consider s deleted
    FileSystem s2;
    std::cout << "Loading..." << std::endl;
    s2.init(fileName);
    
    // check that you can log in
    // note that users will be in a different place in memory
    User* goodCreds2 = s2.login("user02", "password-2");
    std::cout << "Login test: " << goodCreds2 << " (expecting non-zero)" << std::endl;

    std::cout << "Testing is very good. Especially for programs and COVID-19." << std::endl;
};


