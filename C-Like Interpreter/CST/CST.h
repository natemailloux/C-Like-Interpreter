//
// Created by natem on 2/28/2025.
//

#ifndef ASSIGNMENT3_CST_H
#define ASSIGNMENT3_CST_H
#include "../tokenize/token.h"
#include "../symbolTable/symbolTable.h"
#include "node.h"
#include <fstream>

#define IDENTIFIER "IDENTIFIER"
#define STRING "STRING"
#define INTEGER "INTEGER"

void traverseCST(node * head, ofstream& outFS); // used to output the CST

class CST
{
public:

    // constructor/destructor
    CST(token * head, const string& outputFile, symbolTable * ST);
    ~CST() { deleteNodes(head); }

    // insertion/traversal functions
    void insert(node *& parent, node *& curNode, node *& _node, node * lastNode, int origLineNum) const;
    static void insertSibling(node *& parent, node *& curNode, node *& _node, node * lastNode);
    static void insertChild(node *& parent, node *& curNode, node *& _node, node * lastNode);
    void outputTraversal(const string& outputFile);
    void parseEndlToken(token *& iter);
    static node * getLastNode(node * _node);
    static void deleteNodes(node * _node);
    static void parseUntilEndl(token *& iter);

    // parsing functions
    node * parse(token *& iter);
    node * parseSingleQuoteString(token *& iter);
    node * parseDoubleQuoteString(token *& iter);
    node * parseFunction(token *& iter);
    node * parseNumericalExpression(token *& iter);
    node * parseBooleanExpression(token *& iter);
    node * parseExpression(token *& iter);
    node * parseSelectionStatement(token *& iter);
    node * parseIterationStatement(token *& iter);
    node * parseAssignmentStatement(token *& iter);
    node * parsePrintfStatement(token *& iter);
    node * parseGetCharFunction(token *& iter);
    node * parseUserDefinedFunction(token *& iter);
    node * parseUserDefinedFunctionStatement( token *& iter);
    node * parseBracket( token *& iter, node *& parent, node *& child, node *& _node,
                         const string& variableName, const int& origLineNum);
    node * parseDeclarationStatement(token *& iter);
    node * parseReturnStatement(token *& iter);
    node * parseStatement(token *& iter);
    node * parseCompoundStatement(token *& iter);
    node * parseBlockStatement(token *& iter);
    node * parseParameterList(token *& iter);
    node * parseProcedureDeclaration(token *& iter);
    node * parseIdentifierAndIdentifierArrayList(token *& iter);
    node * parseIncrementStatement(token *& iter);
    node * parseFactor(token *& iter);
    node * parseTerm(token *& iter);
    node * root() { return head; }
    symbolTable * createPL(node * iter);

    bool match(token *& iter, const string& expected);
    bool matchType(token *& iter, const string& expected);
    bool isDatatype(token *& iter);
    static bool isBooleanOperator(token *& iter);
    [[nodiscard]] bool hasErrors() const { return errorDetected; }


private:
    node * head;
    string curFunction;
    int lineNum = 1;
    int curScope = 0;
    int prevScope = 0;
    bool errorDetected = false;
    symbolTable * ST;
    ofstream outFS;

};




#endif //ASSIGNMENT3_CST_H
