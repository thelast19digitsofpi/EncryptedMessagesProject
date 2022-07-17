/*
 * main.cpp
 * Croix Gyurek
 *
 * Right now just a testing ground
 */


#include <iostream>
#include "bigNumber.h"
#include "sha.h"
#include "message.h"
#include "user.h"
#include "randomizer.h"
#include "rsaKey.h"
#include "fileSystem.h"


#include <sstream>
void testStringLineifier() {
    std::string testString = "";
    std::stringstream ss1;
    ss1 << testString;
    std::stringstream ss2;
    std::string line;
    while (true) {
        getline(ss1, line);
        // in the actual code I will just make a string for the >>> so it's cleaner
        ss2 << ">>> " << line << std::endl;
        if (ss1.eof()) break;
    }
    std::cout << "[" << ss2.str() << "]" << std::endl;
};

// for testing linked lists
class IntClass {
    public:
        int value;
        IntClass() {
            value = 0;
        };
        IntClass(int v) {
            value = v;
        };
};
void printList(LinkedList<IntClass>& ll) {
    std::cout << "From front: ";
    for (Node<IntClass>* curNode = ll.getHead(); curNode != nullptr; curNode = curNode->getNext()) {
        std::cout << curNode->getPayload()->value << ' ';
    }
    std::cout << std::endl;
    std::cout << "From back: ";
    for (Node<IntClass>* curNode = ll.getTail(); curNode != nullptr; curNode = curNode->getPrev()) {
        std::cout << curNode->getPayload()->value << ' ';
    }
    std::cout << std::endl;
};
void testLinkedList() {
    IntClass* a = new IntClass(1);
    IntClass* b = new IntClass(2);
    IntClass* c = new IntClass(3);
    IntClass* d = new IntClass(4);
    LinkedList<IntClass> list;
    Node<IntClass>* aNode = new Node<IntClass>(a);
    Node<IntClass>* bNode = new Node<IntClass>(b);
    Node<IntClass>* cNode = new Node<IntClass>(c);
    Node<IntClass>* dNode = new Node<IntClass>(d);

    list.insertHead(bNode);
    list.insertHead(aNode);
    list.insertTail(dNode);
    list.insertBefore(cNode, dNode);

    printList(list);

    list.remove(bNode);
    printList(list);

    list.remove(aNode);
    printList(list);

    list.remove(dNode);
    printList(list);
};


int main() {
    std::cout << "Begin test..." << std::endl;
    try {
        std::string args[0];
        FileSystem::main(args);
        //testStringLineifier();
        //FileSystem::test();
        //testSHA();
        //BigNumber::test();
        //testUser();
        //testRandomizer();
        //RSAKey::test();
        //testLinkedList();
    } catch (std::string e) {
        std::cerr << "PANIC Something thrown!!" << std::endl << e << std::endl;
    }
    
    std::cout << "Test complete" << std::endl;
    return 0;
};


