//
// Created by natem on 2/28/2025.
//

#ifndef ASSIGNMENT3_NODE_H
#define ASSIGNMENT3_NODE_H
#include <string>
using namespace std;


class node {

public:
    node(string info, int scope): _content{info}, _scope{scope}, _child{nullptr}, _sibling{nullptr}  {}

    void child(node * _node) { _child = _node; }
    node * child() { return _child; }
    void sibling(node * node) { _sibling = node; }
    node * sibling() { return _sibling; }
    string content() { return _content; }
    int scope() { return _scope; }

private:
    node * _child, * _sibling;
    string _content;
    int _scope;

};


#endif //ASSIGNMENT3_NODE_H
