/*
 * node.h
 * Croix Gyurek
 *
 * Everything is in this file.
 * I will include a redundant .cpp file to make the making easier.
 *
 * everything is a pointer
 * e e y h n   s a p i t r
 *  v r t i g i     o n e
 *
 */

// this may backfire...
// but on second thought how can it?
#ifndef NODE_H_EXISTS
#define NODE_H_EXISTS

// I actually find "T" easier to read than "NodeType"
// because the latter looks too much like Node
template <class T>
class Node {
    protected:
        T* payload;
        Node<T>* prev;
        Node<T>* next;
    public:
        Node();
        // constructor given a payload is useful
        // constructor given links is harder to manage
        Node(T*);
        // warning: deleting a node does NOT delete its neighbors but DOES delete the payload.
        ~Node();

        T* getPayload();
        void setPayload(T*);
        Node<T>* getPrev();
        Node<T>* getNext();
        void setPrev(Node<T>*);
        void setNext(Node<T>*);
};

// have to write the methods in this file... urgh

template <class T>
Node<T>::Node() {
    payload = nullptr;
    next = nullptr;
    prev = nullptr;
};
template <class T>
Node<T>::Node(T* givenPtr) {
    payload = givenPtr;
    next = nullptr;
    prev = nullptr;
};
template <class T>
Node<T>::~Node() {
    delete payload;
    // safety
    if (prev != nullptr && prev->next == this) {
        prev->setNext(nullptr);
    }
    if (next != nullptr && next->prev == this) {
        next->setPrev(nullptr);
    }
};
// getPayload setPayload get/set next/prev
template <class T>
T* Node<T>::getPayload() {
    return payload;
};
template <class T>
void Node<T>::setPayload(T* newPtr) {
    payload = newPtr;
};
template <class T>
Node<T>* Node<T>::getPrev() {
    return prev;
};
template <class T>
Node<T>* Node<T>::getNext() {
    return next;
};
template <class T>
void Node<T>::setPrev(Node<T>* newNode) {
    prev = newNode;
};
template <class T>
void Node<T>::setNext(Node<T>* newNode) {
    next = newNode;
};

#endif
