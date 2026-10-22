//
// Created by natem on 3/10/2025.
//

#include "symbolTable.h"
#include <iostream>
#include <fstream>
#include <cassert>


/***
 * This inserts a symbol table node for parameter lists
 * @param ST the symbol table to be inserted
 */
void symbolTable::insertST(symbolTable * ST)
{
    if (!_next)
    {
        _next = _tail = ST;
    }
    else
    {
        _tail->_next = ST;
        _tail = ST;
    }
    // add to its function node
    addParamList(ST);
}


/***
 * This adds a parameter list pointer to its function symbol table
 * counterpart
 *
 * @param PL the symbol table to be added
 */
void symbolTable::addParamList(symbolTable * PL)
{
    if (_head)
    {
        stNode * iter = _head;
        while (iter)
        {
            if (iter->type() == "function")
            {
                if (iter->idName() == PL->_name)
                {
                    iter->addST(PL);
                    return;
                }
            }
            iter = iter->next();
        }
    }
}


/***
 * This function adds a node to a symbol table list
 *
 * @param curNode the current node in the list
 * @param node the node to be added
 */
void symbolTable::insert(stNode *& node)
{
    if (node) {
        if (!_head) {
            _head = curNode = node;
        } else {
            assert(curNode);
            curNode->next(node);
            curNode = curNode->next();

        }
        node = nullptr;
    }
}


/***
 * This checks a symbol table list to see if a variable/function name has
 * already been used
 *
 * @param name the name to be checked
 * @return true if the name is in use, false if not
 */
bool symbolTable::inUse(stNode * node, int& foundScope)
{
    symbolTable * tableIter = this;
    stNode * iter;
    while(tableIter)
    {
        iter = tableIter->_head;
        // this ensures you are either looking through the main list of variables or
        // the parameter list of the function with the same scope
        if (tableIter->_name.empty() || tableIter->head()->scope() == node->scope()) {
            while (iter) {
                // if the var shares the same name and is in the same scope (or global)
                if (iter->idName() == node->idName() && iter->isArr() == node->isArr() &&
                    (iter->scope() == node->scope() || iter->scope() == 0)) {
                    foundScope = iter->scope(); // used for debug messages
                    return true;
                }
                iter = iter->next();

            }
        }
        tableIter = tableIter->_next;
    }
    return false;
}


/***
 * This function outputs the contents of a Symbol Table in
 * a human readable format
 *
 * @param outputFile the file that will hold the output of this
 * function
 */
void symbolTable::output(const string& outputFile)
{
    stNode * iter;
    symbolTable * tableIter = this;
    ofstream outFS;
    bool paramList = false;

    outFS.open(outputFile);

    if (!outFS.is_open())
    {
        cout << "ERROR OPENING " << outputFile << endl;
        exit(3);
    }

    while (tableIter) {
        iter = tableIter->_head;
        if (!tableIter->_name.empty() && iter)
        {
            outFS << "\n   PARAMETER LIST FOR: " << tableIter->_name << endl;
            paramList = true;
        }
        while (iter)
        {
            outFS << "      IDENTIFIER_NAME: " << iter->idName() << endl;
            if (!paramList)
                outFS << "      IDENTIFIER_TYPE: " << iter->type() << endl;
            outFS << "             DATATYPE: " << iter->dataType() << endl;
            outFS << "    DATATYPE_IS_ARRAY: ";
            if (iter->isArr())
                outFS << "yes\n";
            else
                outFS << "no\n";

            outFS << "  DATATYPE_ARRAY_SIZE: " << iter->size() << endl;
            outFS << "                SCOPE: " << iter->scope() << endl << endl;
            iter = iter->next();
        }
        tableIter = tableIter->_next;
    }

}


/***
 * Destructor
 */
symbolTable::~symbolTable()
{
    deleteNodes();
}


/***
 * This function iterates through an assortment of symbol tables (and their LL)
 * to delete all the nodes and STs
 */
void symbolTable::deleteNodes()
{
    symbolTable * tableIter = this, *tempTable;
    stNode * iter, *temp;
    while(tableIter)
    {
        iter = tableIter->_head;
        while (iter)
        {
            temp = iter;
            iter = iter->next();
            delete temp;
        }
        tempTable = tableIter;
        tableIter = tableIter->_next;
        delete tempTable;
    }
    _head = nullptr;
}


/***
 * This retrieves a node from a symbol table (generally used for AST nodes)
 * @param name name of the variable
 * @param scope scope of the variable
 * @return pointer to the STnode if it exists
 */
stNode * symbolTable::retrieveNode(const string& name, const int& scope)
{
    symbolTable * tableIter = this;
    stNode * iter;
    while(tableIter)
    {
        iter = tableIter->_head;
        if (tableIter->_name.empty() || tableIter->head()->scope() == scope) {
            while (iter) {
                if (iter->idName() == name && (iter->scope() == scope || iter->type() == "function" || iter->scope() == 0)) {
                    return iter;
                }
                iter = iter->next();

            }
        }
        tableIter = tableIter->_next;
    }
    return nullptr;
}



/***
 * This checks if a variable exists
 * @param name name of the variable
 * @param scope scope of the variable
 * @return pointer to the STnode if it exists
 */
bool symbolTable::exists(const string& name, const int& scope)
{
    symbolTable * tableIter = this;
    stNode * iter;
    while(tableIter)
    {
        iter = tableIter->_head;
        if (tableIter->_name.empty() || tableIter->head()->scope() == scope) {
            while (iter) {
                if (iter->idName() == name && (iter->scope() == scope) || (iter->scope() == 0)) {
                    return true;
                }
                iter = iter->next();

            }
        }
        tableIter = tableIter->_next;
    }
    return false;
}


/***
 * This assigns a value to a function's parameter list
 * @param function the given function
 * @param val the value to be assigned
 * @param varNum the index of the variable in the param list
 */
void symbolTable::assignCallParam(const string& function, const string& val, int varNum)
{
    symbolTable * tableIter = this;
    stNode * iter;
    while (tableIter)
    {
        if (tableIter->_name == function)
        {
            iter = tableIter->_head;
            for (int i = 0; i < varNum; i++)
            {
                iter = iter->next();
            }
            if (!iter)
                throw runtime_error("Invalid amount of parameters added\n");
            iter->value = val;
            return;
        }
        tableIter = tableIter->_next;
    }
}




