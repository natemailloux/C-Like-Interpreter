//
// Created by natem on 4/4/2025.
//

#include <cassert>
#include <iostream>
#include "AST.h"
#include <stack>
using namespace std;


/**
 * Constructor for the AST using a previously constructed Concrete
 * Syntax Tree and Symbol Table
 */
AST::AST(CST *cst, symbolTable * ST)
{
    // initialize the iterator node to the root of the CST
    // and insertion pointers
    node * iter = cst->root();
    _main = nullptr;
    ASTnode * parent = nullptr, *child = nullptr, * _node = parent;
    this->ST = ST;

    while (iter)
    {
        // variable used to check if insertions are for a sibling/child
        int origLineNum = lineNum;

        // check if the current node is for a function/procedure
        if (match(iter, "function") || (match(iter, "procedure")))
        {
            child = parseFunctionDeclaration(iter);
        }
        // if not, you are currently traversing a global variable
        else if (isDatatype(iter))
        {
            child = parseVarDeclaration(iter);
        }
        // this should not be called and used for a debugging tool
        else
        {
             nextNode(iter);
             printf("INVALID STATEMENT ENCOUNTERED\n");
        }
        // after creating the new child node, insert it into the AST
        insert(parent, _node, child, origLineNum);
    }
    // set the head to the parent node
    head = parent;
}


/***
 * Destructor
 */
AST::~AST()
{
    ASTnode * iter = head;
    while (iter)
    {
        ASTnode * temp = iter;
        if (iter->sibling())
            iter = iter->sibling();
        else
            iter = iter->child();
        delete temp;
    }
}



/***
 * This is the _main insertion function for the AST
 * @param parent the parent/root node
 * @param _node the current place in the AST
 * @param child the node to be inserted
 * @param origLineNum this is used to see if the line number
 * had changed during the creation of the child node
 */
void AST::insert(ASTnode *& parent,  ASTnode *& _node, ASTnode *& child, int origLineNum)
{
    // ensure that the child node is not null
    if (child) {
        // if you stayed on the same line, insert as a sibling
        if (origLineNum == lineNum)
            insertSibling(parent, _node, child);
        // line number changed, insert as a child of the AST
        else
            insertChild(parent, _node, child);
    }
}


/***
 * This function insert a node as a child of a current AST structure
 *
 * @param parent parent/root node of the structure
 * @param curNode the last/end node in the structure
 * @param _node the root node to be inserted
 */
void AST::insertChild(ASTnode *& parent, ASTnode *& curNode, ASTnode *& _node )
{
    // check if the insertion node is null
    if (!_node)
        return;

    // check if the tree is empty
    if (!parent)
    {
        parent = _node;
        curNode = getLastNode(_node);
    }
    else
    {
        // tree is not empty, set the current node's child to the
        // new node and set current node to the last node of the
        // child node
        assert(curNode);
        curNode->child(_node);
        curNode = getLastNode(_node);
    }
    // set the child node ptr to null
    _node = nullptr;
}



/***
 * This is used to insert a new node as a sibling of a current
 * AST structure
 *
 * @param parent parent/root node of the structure
 * @param curNode the last/end node in the structure
 * @param _node the root node to be inserted
 */
void AST::insertSibling(ASTnode *& parent, ASTnode *& curNode, ASTnode *& _node )
{
    // ensure the insertion node is not null
    if (!_node)
        return;

    // check if the tree is currently empty
    if (!parent)
    {
        parent = _node;
        curNode = getLastNode(_node);
    }
    else
    {
        // tree is not empty so set the sibling of the current node
        // to the child
        // then set current node to the last node of the sibling
        assert(curNode);
        curNode->sibling(_node);
        curNode = getLastNode(_node);
    }
    // set the child node ptr to null
    _node = nullptr;
}


/***
 * This safely moves the iterator pointer to
 * the next node in the CST
 * @param iter the pointer to the current place
 * in the CST
 */
void AST::nextNode(node *& iter)
{
    // check if the given pointer is null
    if (iter) {
        // if it has a sibling iterate to it
        if (iter->sibling())
            iter = iter->sibling();
        // iterate to its child if it does not
        else {
            iter = iter->child();
            // increment lineNum so we know we have
            // changed lines
            lineNum++;
        }
    }
}


/***
 * This checks if a current node it a identifier node holding
 * a supported datatype
 *
 * @param iter the node pointer
 * @return true if it is, false if not
 */
bool AST::isDatatype(node * iter)
{
    // ensure the node is not null, so you do not try to call
    // a member function on a null pointer
    if (iter)
    {
        if (iter->content() == "int" || iter->content() == "char" || iter->content() == "bool")
            return true;
    }
    return false;
}


/***
 * This function safely checks if a given node holds a
 * set string value of any type
 *
 * @param iter the current node in the CST
 * @param str the string to be compared
 * @return true if the node contains that set string,
 * false if not
 */
bool AST::match(node * iter, const string& str)
{
    // if the node is not null and holds the string value
    if (iter && iter->content() == str)
    {
        return true;
    }
    return false;
}


/***
 * This function parses a variable declaration statement
 *
 * @param iter the current node in the CST
 * @return the root of the AST structure
 */
ASTnode * AST::parseVarDeclaration(node *& iter)
{
    // initialize the line number variable (for insertions)
    int origLineNum = lineNum;
    // iterate past the datatype to the name of the variable
    iter = iter->sibling();

    // create a new Declaration AST node with a pointer to the variable's symbol table node
    auto * parent = new ASTnode(DECLARATION, ST->retrieveNode(iter->content(), iter->scope()));

    ASTnode * child = nullptr, * _node = parent;
    iter = iter->sibling();

    // if you have a variable that is an array, parse past the brackets
    if (match(iter, "["))
    {
        iter = iter->sibling(); // iterate past first bracket
        if (!match(iter, "]")) // if there is a size variable
            iter = iter->sibling();
        iter = iter->sibling(); // pass ending bracket
    }

    // check if there are multiple declarations
    if (match(iter, ","))
    {
        iter = iter->sibling();
        while (iter && !match(iter, ";"))
        {
            lineNum++; // use if you want multiple declarations on the same line to appear as children
            if (!match(iter, ",")) {
                if (match(iter, "["))
                {
                    iter = iter->sibling(); // iterate past the first bracket
                    iter = iter->sibling(); // iterate past the index variable
                    iter = iter->sibling(); // iterate past the end bracket
                    if (match(iter, ";"))
                        break;
                }
                child = new ASTnode(DECLARATION, ST->retrieveNode(iter->content(), iter->scope()));
                insert(parent, _node, child, origLineNum);
            }
            iter = iter->sibling();
        }
    }

    // iterate past ";"
    nextNode(iter);
    return parent;

}


/***
 * This function parses a function/procedure declaration
 *
 * @param iter the current node in the CST
 * @return the AST structure of the function/procedure
 */
ASTnode * AST::parseFunctionDeclaration(node *& iter)
{
    // set the origLineNum for future insertion
    int originalLineNum = lineNum;
    int scope = iter->scope();

    // check if function
    if (iter->content() == "function")
        iter = iter->sibling(); // iterate to return type
    iter = iter->sibling(); // iterate to name

    string funcName = iter->content();
    auto * declarationNode = new ASTnode(DECLARATION, ST->retrieveNode(funcName, scope));
    if (funcName == "main")
        _main = declarationNode;
    auto * _node = declarationNode;
    functionPtrs.push_back(declarationNode); // add the function node to the array

    // iterate past the function parameters
    parseToGivenChar(")", iter);
    // iterate past ending ")"
    nextNode(iter);

    // create a child of the contents of the function/procedure
    auto * child = parseBlockStatement(iter);
    // insert it
    insert(declarationNode, _node, child, originalLineNum);
    return declarationNode;
}


/***
 * This function parses a block statement for conditions
 * and functions/procedures
 *
 * @param iter the current node in the CST
 * @return the root node of the block statement in an
 * AST structure
 */
ASTnode* AST::parseBlockStatement( node *& iter) {

    // set initial variables and parent node
    int origLineNum = lineNum;
    auto * parent = new ASTnode(BEGIN_BLOCK, nullptr);
    auto * _node = parent;
    nextNode(iter);

    // parse the statements contained in the block
    ASTnode* body = parseCompoundStatement(iter);
    insert(parent, _node, body, origLineNum);

    // add the ending bracket
    auto * child = new ASTnode(END_BLOCK, nullptr);

    // insert the end bracket and iterate to the next node
    insert(parent, _node, child, origLineNum);
    nextNode(iter);

    return parent;
}


/***
 * This parses to a set char from a given node
 *
 * @param character the character to parse to
 * @param iter the current node in the CST
 */
void AST::parseToGivenChar(const string& character, node *& iter)
{
    if (iter)
    {
        while (iter && iter->content() != character)
        {
            if (iter->sibling())
                iter = iter->sibling();
            else if (iter->child()) {
                iter = iter->child();
                lineNum++;
            }
            else
                break;
        }
    }
}


/***
 * This function parses a compound statement
 *
 * @param iter current CST node
 * @return the root node of the AST structured
 * compound statement
 */
ASTnode* AST::parseCompoundStatement( node *& iter ) {

    int origLineNum = lineNum;
    ASTnode * parent = nullptr;
    ASTnode * _node = parent;
    ASTnode * child = nullptr;

    // while you do not encounter the ending bracket
    // parse the inside statements
    while (iter && iter->content()!= "}")
    {
        // parse the current statement and insert it
        child = parseStatement(iter);
        insert(parent, _node, child, origLineNum);
        // parse the semicolon if you are currently at one
        if (iter && iter->content() == ";")
        {
            nextNode(iter);
        }
    }
    return parent;
}


/***
 * This parses an AST structure and returns the
 * last node
 *
 * @param root the root node of the structure
 * @return the last node in the structure
 */
ASTnode* AST::getLastNode(ASTnode * root) {

    if (!root)
        return nullptr;

    auto * iter = root;
    while (iter) {

        while (iter->sibling())
            iter = iter->sibling();

        if (iter->child()) {
            iter = iter->child();
        }
        else {
            break;
        }
    }
    return iter;
}


/***
 * This function parses a return statement
 *
 * @param iter the current node in the CST
 * @return root node of the created structure of the
 * return statement
 */
ASTnode * AST::parseReturnStatement(node *& iter)
{
    // set the origLineNum for safety if return statement
    // takes multiple lines
    int origLineNum = lineNum;
    auto * parent = new ASTnode("RETURN", nullptr);
    auto * _node = parent;
    iter = iter->sibling(); // iterate to the first node of the return expression

    // convert the return expression to post and insert it
    auto * child = convertToPostFix(iter);
    insert(parent, _node, child, origLineNum);
    nextNode(iter); // iterate past the semicolon
    return parent;
}


/***
 * Parse a double quote string (very similar to single quote string)
 *
 * @param iter current CST node
 * @return AST structure of the string
 */
ASTnode * AST::parseString(node *& iter)
{
    int origLineNum = lineNum;
    auto * parent = new ASTnode(iter->content(), nullptr); // catch initial quote
    auto * _node = parent;
    ASTnode * child = nullptr;
    iter = iter->sibling();

    // get the inside string if it exists
    if (!match(iter, "\"") && !match(iter, "\'"))
    {
        child = new ASTnode(iter->content(), nullptr);
        insert(parent, _node, child, origLineNum);
        iter = iter->sibling();
    }

    // get the ending quote
    child = new ASTnode(iter->content(), nullptr);
    insert(parent, _node, child, origLineNum);
    iter = iter->sibling();
    return parent;

}


/***
 * This parses a printf statement
 * @param iter current CST node
 * @return AST structure of the statement
 */
ASTnode * AST::parsePrintfStatement(node *& iter)
{
    // initialize line flag and parent node
    int origLineNum = lineNum;
    auto * parent = new ASTnode("printf", nullptr);
    auto * _node = parent;
    ASTnode * child = nullptr;

    iter = iter->sibling(); // iterate to (
    iter = iter->sibling(); // iterate to quote
    iter = iter->sibling(); // iterate past quote

    child = new ASTnode(iter->content(), nullptr); // catch string
    insert(parent, _node, child, origLineNum);
    iter = iter->sibling(); // iterate to end quote
    iter = iter->sibling(); // iterate past

    // catch variables after the string if they exist
    while (iter && iter->content() != ")")
    {
        // add non-comma nodes to the structure
        if (!match(iter, ","))
        {
            child = new ASTnode(iter->content(), ST->retrieveNode(iter->content(), iter->scope()));
            insert(parent, _node, child, origLineNum);
        }
        iter = iter->sibling();

    }
    // iterate past )
    nextNode(iter);
    return parent;

}


/***
 * This checks a given node to see what type of statement
 * needs to be parses
 *
 * @param iter current CST node
 * @return AST structure of the statement
 */
ASTnode* AST::parseStatement(node *& iter) {

    // conditional
    if (match(iter, "if"))
    {
        return parseSelectionStatement(iter);
    }
    // block statement
    else if (match(iter, "{"))
    {
        return parseBlockStatement(iter);
    }
    // for/while loop
    else if (match(iter,  "while") ||
             match(iter,  "for"))
    {
        return parseIterationStatement(iter);
    }
    // return statement
    else if (match(iter,  "return"))
    {
        return parseReturnStatement(iter);
    }
    // printf statement
    else if (match(iter,  "printf")) {
        return parsePrintfStatement(iter);
    }
    // variable declaration
    else if (isDatatype(iter)) {
        return parseVarDeclaration(iter);
    }
    // other identifier
    else  {

        node * peek = iter->sibling();
        // check if it is an assignment statement
        if (match(peek, "=") || match(peek, "["))
            return parseAssignmentStatement(iter);
        // not assignment, so it must be a user function call
        else
            return parseUserDefinedFunction(iter);
    }

}


/***
 * This parses "for" and "while" statements
 * @param iter current CST node
 * @return AST structure of the entire statement
 */
ASTnode * AST::parseIterationStatement(node *& iter)
{
    // set line number flag and node pointers
    int origLineNum = lineNum;
    ASTnode * parent = nullptr, * _node = parent, * child = nullptr;

    // check if you have encountered a while loop
    if (match(iter, "while")) {
        // catch initial node
        parent = new ASTnode("while", nullptr);
        _node = parent;
        iter = iter->sibling();

        // convert the condition to post fix
        ASTnode* condition = convertToPostFix(iter);
        insert(parent,_node, condition, origLineNum);
        nextNode(iter);

        // check if there is a block statement after the condition
        if (match(iter, "{")) {
            ASTnode* block = parseBlockStatement(iter);
            insert(parent, _node, block, origLineNum);
        }
        // regular statement
        else {
            ASTnode* stmt = parseStatement(iter);
            insert(parent,_node, stmt, origLineNum);
        }
        return parent;
    }

    if (match(iter, "for")) {

        iter = iter->sibling();

        // initialization expression
        parent = new ASTnode("FOR EXPRESSION 1", nullptr);
        _node = parent;
        iter = iter->sibling();

        ASTnode* init = convertToPostFix(iter);
        insert(parent, _node, init, origLineNum);
        lineNum++;

        // condition
        child = new ASTnode("FOR EXPRESSION 2", nullptr);
        insert(parent, _node, child, origLineNum);
        iter = iter->sibling();
        origLineNum = lineNum;

        ASTnode* condition = convertToPostFix(iter);
        insert(parent,_node, condition, origLineNum);
        nextNode(iter);
        lineNum++;

        // iteration statement
        child = new ASTnode("FOR EXPRESSION 3", nullptr);
        insert(parent, _node, child, origLineNum);
        origLineNum = lineNum;

        ASTnode* update = convertToPostFix(iter);
        insert(parent,_node, update, origLineNum);
        origLineNum = lineNum;
        lineNum++;

        // iterate to next node
        nextNode(iter);

        if (match(iter, "{")) {
            ASTnode* block = parseBlockStatement(iter);
            insert(parent,_node, block, origLineNum);
        } else {
            ASTnode* stmt = parseStatement(iter);
            insert(parent,_node, stmt, origLineNum);
        }

        return parent;
    }
    return nullptr; // should not get called
}


/***
 * This parses a user function call
 *
 * @param iter current CST node
 * @return the AST structure of the call
 */
ASTnode * AST::parseUserDefinedFunction(node *& iter)
{
    int origLineNum = lineNum;
    string prev = iter->content();

    // create a flag node for the call
    auto * parent = new ASTnode("call", nullptr);
    auto * child = new ASTnode(iter->content(), ST->retrieveNode(prev, iter->scope()));
    auto * _node = parent;
    insert(parent, _node, child, origLineNum); // connect call node with function name
    iter = iter->sibling(); // iterate to the first (

    while (iter && prev != ")") {
        // insert non-comma nodes
        if ( !match(iter, ",")) {
            child = new ASTnode(iter->content(), ST->retrieveNode(iter->content(), iter->scope()));
            insert(parent, _node, child, origLineNum);
        }
        prev = iter->content();
        iter = iter->sibling();
    }
    nextNode(iter);
    return parent;
}


/***
 * This parses an assignment statement
 * @param iter
 * @return
 */
ASTnode * AST::parseAssignmentStatement(node *& iter)
{
    // create a parent node with a flag node
    auto * assignmentNode = new ASTnode(ASSIGNMENT, nullptr);
    string varName = iter->content();
    auto * _node = assignmentNode;
    // convert the entire assignment statement to post fix and insert it
    auto * child = convertToPostFix(iter);
    child->node(ST->retrieveNode(varName, iter->scope()));
    insert(assignmentNode, _node, child, lineNum);
    return assignmentNode;
}


/***
 * This parses a selection statement (conditions)
 * @param iter the current place in the CST
 * @return an AST structure of the selection statement
 */
ASTnode * AST::parseSelectionStatement(node *& iter)
{
    // set parent node and related parameters
    int origLineNum = lineNum;
    auto * parent = new ASTnode("if", nullptr);
    auto * _node = parent;
    ASTnode* child = nullptr;
    iter = iter->sibling(); // iterate to first node of the expression

    // convert the condition to post fix
    child = convertToPostFix(iter);
    insert(parent, _node, child, origLineNum);
    origLineNum = lineNum;
    nextNode(iter);

    // if the condition has a block statement
    if (match(iter, "{")) {
        ASTnode* block = parseBlockStatement(iter);
        insert(parent, _node, block ,origLineNum);
    }
    // single line statement
    else {
        ASTnode* stmt = parseStatement(iter);
        insert(parent,_node, stmt,origLineNum);
    }


    // is there an else after the if statement(s)
    if (match(iter, "else")) {

        // set the parent node and iterate to the next node
        child = new ASTnode("else", nullptr);
        insert(parent, _node, child, origLineNum);
        origLineNum = lineNum;
        nextNode(iter);

        // else statement has a block statement
        if (match(iter, "{")) {
            ASTnode* block = parseBlockStatement(iter);
            insert(parent,_node, block,origLineNum);
        }
        // it only has a single line statement
        else {
            ASTnode* stmt = parseStatement(iter);
            insert(parent,_node, stmt,origLineNum);
        }
    }

    return parent;
}


/***
 * This function checks if a node contains only an operator
 * @param iter current CST node
 * @return true if the node contains an operator, false if not
 */
bool AST::isOperator(node * iter)
{
    return (iter->content() == "+" || iter->content() == "-" || iter->content() == "*" ||
            iter->content() == "/" || iter->content() == "%" || iter->content() == "!" ||
            iter->content() == "&&" || iter->content() == "||" || iter->content() == "==" ||
            iter->content() == "!=" || iter->content() == "<="  || iter->content() == ">=" ||
            iter->content() == "<" || iter->content() == ">" || iter->content() == "=" );
}


/***
 * This function checks if a given string contains only an operator
 * @param iter current string
 * @return true if the string contains only an operator, false if not
 */
bool AST::isOpString(const string& iter)
{
    return (iter == "+" || iter == "-" || iter == "*" ||
            iter == "/" || iter == "%" || iter == "!" ||
            iter == "&&" || iter == "||" || iter == "==" ||
            iter == "!=" || iter == "<="  || iter == ">=" ||
            iter == "<" || iter == ">" || iter == "=" );
}


/***
 * This returns the precedence of a given operator
 * (lower the value the more priority)
 * @param op the given operator
 * @return the given value of their precedence
 */
int AST::precedence(const string& op)
{
    if (op == "!")
        return 1;
    else if (op == "*" || op == "/" || op == "%")
        return 2;
    else if (op == "+" || op == "-")
        return 3;
    else if (op == "<" || op == ">" || op == ">=" || op == "<=")
        return 4;
    else if (op == "==" || op == "!=")
        return 5;
    else if (op == "&&")
        return 6;
    else if (op == "||")
        return 7;
    else
        return 8; // assignment
}


/***
 * Checks if a given operator is left associative
 * @param op the given operator
 * @return true if it is left-associative, false if not
 */
bool AST::isLeftAssociative(const string& op)
{
    if (op == "!" || op == "=" || op == "+=" || op == "-=") {
        return false;
    }
    return true;
}


/***
 * This converts a set expression and converts it to post fix
 *
 * @param iter current CST node
 * @return the head node of the converted expression
 */
ASTnode * AST::convertToPostFix(node *& iter)
{
    stack<string> opStack;
    ASTnode * parent = nullptr, *child = nullptr;
    auto * _node = parent;

    int origLineNum = lineNum;
    while (iter && iter->sibling() && !match(iter, "{") && !match(iter, ";"))
    {
        // non-operator
        auto * peek = iter->sibling();
        if (!isOperator(iter) && !match(iter, "(") && !match(iter, ")") )
        {

            if ((match(iter, "'")) || (match(iter, "\"")))
            {
                child = parseString(iter);
                insert(parent, _node, child, origLineNum);
                continue;
            }
            else {
                // user function
                child = new ASTnode(iter->content(), ST->retrieveNode(iter->content(), iter->scope()));
            }
            insert(parent, _node, child, origLineNum);

            // user-defined function
            string prev;
            if (match(peek, "("))
            {
                iter = iter->sibling();
                while (iter && prev != ")") {

                    if (!match(iter, ",")) {
                        child = new ASTnode(iter->content(), ST->retrieveNode(iter->content(), iter->scope()));
                        insert(parent, _node, child, origLineNum);
                    }
                    prev = iter->content();
                    iter = iter->sibling();
                }
                continue;

            }
        }
        // operator case
        else if (isOperator(iter))
        {
            string curOp = iter->content();
            int curPrecedence = precedence(curOp);

            while (!opStack.empty() && isOpString(opStack.top()) &&
            (curPrecedence > precedence(opStack.top())
                || (curPrecedence == precedence(opStack.top()) && isLeftAssociative(curOp)) ))
            {
                child = new ASTnode(opStack.top(), nullptr);
                insert(parent, _node, child, origLineNum);
                opStack.pop();
            }
            opStack.push(curOp);
        }
        else if (match(iter, "("))
        {
            opStack.emplace("(");
        }
        else if (match(iter, ")"))
        {

            while (opStack.top() != "(")
            {
                child = new ASTnode(opStack.top(), nullptr);
                insert(parent, _node, child, origLineNum);
                opStack.pop();
            }
            opStack.pop();
        }
        nextNode(iter);
        origLineNum = lineNum;
    }

    while (!opStack.empty())
    {
        if (opStack.top() != "(" && opStack.top() != ")") {
            child = new ASTnode(opStack.top(), nullptr);
            insert(parent, _node, child, origLineNum);
        }
        opStack.pop();
    }
    return parent;

}


/***
 * This function looks through the vector of ASTnode pointers
 * (of the first node of functions) and returns the first node
 * of a function based on the given name (if it exits)
 *
 * @param name the name of the function call
 * @return the first node of that function or null if the function
 * does note exist (should not be called on non-functions)
 */
ASTnode * AST::retrieveFunctionPtr(const string& name)
{
    for (auto * i : functionPtrs)
    {
        if (i->node()->idName() == name)
            return i;
    }
    return nullptr;
}


/***
 * This outputs the contents of a CST in a human-readable way
 * @param outputFile file to hold the contents of the CST
 */
void AST::output(const string& outputFile)
{
    ofstream outFS;
    outFS.open(outputFile);

    if (!outFS.is_open())
    {
        std::cout << "ERROR OPENING " << outputFile << endl;
        exit(3);
    }

    traverseAST(head,outFS);
    if (head)
        outFS << "null";

    outFS.close();

}


#include <iomanip>
#define ORIGINAL_LENGTH 1
int lineWid = ORIGINAL_LENGTH;


/***
 * This traverse the AST and outputs the tokens in their tree
 * structure
 * @param head root node of the tree
 * @param outFS output file stream
 */
void AST::traverseAST(ASTnode * head, ofstream& outFS)
{
//    if (!head)
//        return;
//    outFS << "[" << head->content() << "]->";
//    if (head->sibling())
//        lineWid += head->content().size() + 4;
//    traverseAST(head->sibling(), outFS);
//
//    if (head->child())
//    {
//        outFS << "null\n" << setw(lineWid);
//        traverseAST(head->child(), outFS);
//        lineWid = ORIGINAL_LENGTH;
//
//    }
    if (!head)
        return;
    outFS << "[" << head->content() << "]->";
//    if (head->sibling())
//        lineWidth += head->content().size() + 4;
//    outFS << head->content() << " ";
//    if (head->sibling())
//        lineWidth += head->content().size();

    traverseAST(head->sibling(), outFS);

    if (head->child()) {
        outFS << "null\n\n" << setw(lineWid);
        traverseAST(head->child(), outFS);
        lineWid = ORIGINAL_LENGTH;
    }

}