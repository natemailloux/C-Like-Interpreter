//
// Created by natem on 4/4/2025.
//

#ifndef ASSIGNMENT4_ASTNODE_H
#define ASSIGNMENT4_ASTNODE_H
#include <string>
#include <utility>
#include "../symbolTable/stNode.h"
using namespace std;


class ASTnode {

public:

    ASTnode(string title, stNode * _node): _sibling{nullptr}, _child{nullptr},
            _title{std::move(title)}, _node{_node} {}
    ASTnode(string title, string datatype): _sibling{nullptr}, _child{nullptr},
    _title{std::move(title)}, _type{std::move(datatype)}, _node{nullptr} {}

    ASTnode * sibling() { return _sibling; }
    ASTnode * child() { return _child; }
    stNode * node() { return _node; }

    string type() { return _type; }
    string content() { return _title; }

    void sibling(ASTnode * astNode) { _sibling = astNode; }
    void child(ASTnode * astNode) { _child = astNode; }
    void node(stNode * node) { _node = node; }
    void type(const string& str) { _type = str; }


private:
    string _title;
    string _type;
    ASTnode * _sibling, * _child;
    stNode * _node;
};


#endif //ASSIGNMENT4_ASTNODE_H
