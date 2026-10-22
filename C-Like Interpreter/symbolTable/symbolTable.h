//
// Created by natem on 3/10/2025.
//

#ifndef ASSIGNMENT4_SYMBOLTABLE_H
#define ASSIGNMENT4_SYMBOLTABLE_H
#include <utility>

#include "stNode.h"
#include "../tokenize/tokenList.h"
#include "../tokenize/tokenize.h"


class symbolTable {

public:
    symbolTable(): _head{nullptr}, curNode{nullptr}, _next{nullptr}, _tail{nullptr} {}
    explicit symbolTable(string name): _name{std::move(name)}, _head{nullptr}, curNode{nullptr}, _next{nullptr},
    _tail{nullptr} {}
    ~symbolTable();

    void insert( stNode *& _token);
    void insertST(symbolTable * ST);
    void output(const string& outputFile);
    void deleteNodes();
    void addParamList(symbolTable * PL);
    void assignCallParam(const string& function, const string& val, int varNum);

    bool inUse(stNode * node, int& foundScope);
    bool exists(const string& name, const int& scope);

    stNode * head() { return _head; }
    stNode * retrieveNode(const string& name, const int& scope);

private:
    stNode * _head, * curNode;
    string _name;
    symbolTable * _next, * _tail;

};


#endif //ASSIGNMENT4_SYMBOLTABLE_H
