//
// Created by natem on 3/10/2025.
//

#ifndef ASSIGNMENT3_TOKENLIST_H
#define ASSIGNMENT3_TOKENLIST_H
#include "token.h"


class tokenList {

public:
    tokenList(): _head{nullptr}, curToken{nullptr} {}
    ~tokenList() { deleteNodes(); }
    void insert(token *& _token);
    token * head() { return _head; }
    void output(const string& outputFile);
    void deleteNodes();

private:
    token * _head, *curToken;

};


#endif //ASSIGNMENT3_TOKENLIST_H
