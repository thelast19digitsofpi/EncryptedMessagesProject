/*
 * linkedList.h
 * Croix Gyurek
 *
 * Everything is here.
 * The .cpp file just includes the .h file and gets on with it.
 *
 * Technically a double linked list.
 */

#ifndef LINKEDLIST_H_EXISTS
#define LINKEDLIST_H_EXISTS

#include "node.h"

template <class T>
class LinkedList {
    protected:
        Node<T>* head;
        Node<T>* tail;
    public:
        LinkedList();
        ~LinkedList();
        
        // boring getters
        Node<T>* getHead();
        Node<T>* getTail();

        // append nodes
        void insertHead(Node<T>*);
        void insertTail(Node<T>*);
        void insertBefore(Node<T>*, Node<T>*);
        void remove(Node<T>*);
        Node<T>* getFromHead(int);
        Node<T>* getFromTail(int);
        int getSize();
};

template <class T>
LinkedList<T>::LinkedList() {
    head = nullptr;
    tail = nullptr;
};

template <class T>
LinkedList<T>::~LinkedList() {
    Node<T>* curNode = head;
    while (curNode != nullptr) {
        Node<T>* next = curNode->getNext();
        delete curNode;
        curNode = next;
    }
};

template <class T>
Node<T>* LinkedList<T>::getHead() {
    return head;
};

template <class T>
Node<T>* LinkedList<T>::getTail() {
    return tail;
};

template <class T>
void LinkedList<T>::insertHead(Node<T>* newNode) {
    newNode->setNext(head);
    if (head != nullptr) {
        head->setPrev(newNode);
    }
    head = newNode;
    if (tail == nullptr) {
        // list was empty
        tail = newNode;
    }
};

template <class T>
void LinkedList<T>::insertTail(Node<T>* newNode) {
    newNode->setPrev(tail);
    if (tail != nullptr) {
        tail->setNext(newNode);
    }
    tail = newNode;
    if (head == nullptr) {
        head = newNode;
    }
};

template <class T>
void LinkedList<T>::remove(Node<T>* deadNode) {
    if (deadNode != nullptr) {
        Node<T>* before = deadNode->getPrev();
        Node<T>* after = deadNode->getNext();
        if (deadNode == head) {
            // removing the head node
            head = after;
            if (after != nullptr) {
                after->setPrev(nullptr);
            }
        } else {
            // not the head
            before->setNext(after);
            // the other direction is done in the second part
        }
    
        if (deadNode == tail) {
            tail = before;
            if (before != nullptr) {
                before->setNext(nullptr);
            }
        } else {
            // not the tail
            after->setPrev(before);
        }
        // If you really want to preserve the payload just setPayload(0) first
        delete deadNode;
    }
};

template <class T>
void LinkedList<T>::insertBefore(Node<T>* newNode, Node<T>* beforeMe) {
    Node<T>* prev = beforeMe->getPrev();
    if (prev == nullptr) {
        // insert before the first node
        head = newNode;
        newNode->setPrev(nullptr);
    } else {
        // insert between beforeMe and its prev
        prev->setNext(newNode);
        newNode->setPrev(prev);
    }
    newNode->setNext(beforeMe);
    beforeMe->setPrev(newNode);
};

template <class T>
Node<T>* LinkedList<T>::getFromHead(int n) {
    Node<T>* curNode = head;
    for (int i = 0; i < n; i++) {
        if (curNode != nullptr) {
            curNode = curNode->getNext();
        }
    }
    return curNode;
};

template <class T>
Node<T>* LinkedList<T>::getFromTail(int n) {
    Node<T>* curNode = tail;
    for (int i = 0; i < n; i++) {
        if (curNode != nullptr) {
            curNode = curNode->getPrev();
        }
    }
    return curNode;
};

template <class T>
int LinkedList<T>::getSize() {
    Node<T>* curNode = head;
    int count = 0;
    while (curNode != nullptr) {
        curNode = curNode->getNext();
        count++;
    }
    return count;
};


#endif
