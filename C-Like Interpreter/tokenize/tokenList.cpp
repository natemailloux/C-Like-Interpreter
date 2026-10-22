//
// Created by natem on 3/10/2025.
//

#include "tokenList.h"
#include <cassert>
#include <iostream>
#include <fstream>

/***
 * This function adds a token to a list
 * @param curToken the current place in the list
 * @param _token the token to be added
 */
void tokenList::insert(token *& _token)
{
    if (_token) {
        if (!_head) {
            _head = _token;
            curToken = _token;
        } else {
            assert(curToken);
            curToken->next(_token);
            curToken = curToken->next();
        }
        _token = nullptr;
    }

}


/***
 * This deletes all the nodes within a token list
 */
void tokenList::deleteNodes()
{
    token * iter = _head, * temp;
    while (iter)
    {
        temp = iter;
        iter = iter->next();
        delete temp;
    }
}




/***
 * This is used for debugging purposes to see whether a token list is properly
 * constructed
 * @param outputFile file where the output is held
 * @param head first token of the token list
 */
void tokenList::output(const string& outputFile)
{

    ofstream outFS;
    outFS.open(outputFile);

    if (!outFS.is_open())
    {
        cout << "ERROR OPENING " << outputFile << endl;
        exit(3);
    }

    token * iter = _head;
    outFS << "\nToken list:\n\n";
    while (iter)
    {
        outFS << "Token type: " << iter->_type() << endl;
        outFS << "Token:      " << iter->content() << endl << endl;
        // token * temp = iter;
        iter = iter->next();
        // delete temp;
    }

    outFS.close();
//    cout << "SUCCESSFUL\n";

}