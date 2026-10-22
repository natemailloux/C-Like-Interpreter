//
// Originally created by Nathan Mailloux on 2/28/2025.
//


#include "CST.h"
#include <cassert>
#include <iostream>
#include <utility>
#include <fstream>
using namespace std;


/***
 * This is the main constructor for the CST
 * @param tokenHead head token node
 * @param outputFile the output file that will hold errors if encountered
 * @param ST the symbol table to be populated throughout the process
 * of constructing the CST
 */
CST::CST(token * tokenHead, const string& outputFile, symbolTable * ST)
{
    this->ST = ST;

    // clear the file
    outFS.open(outputFile);
    outFS.close();

    outFS.open(outputFile);

    if (!outFS.is_open())
        throw runtime_error("CST FILE DID NOT OPEN\n");

    head = parse(tokenHead);

    outFS.close();

}


/***
 * This is the main insertion function that uses two helper functions
 * (insertSibling and insertChild) depending on if the line number changed
 * during the creation of the child node
 *
 * @param parent current parent node in the structure
 * @param curNode current place in the structure
 * @param _node the child node to be inserted
 * @param lastNode the last node of the child node structure
 * @param origLineNum the original line number prior to the creation of the child node
 */
void CST::insert(node *& parent, node *& curNode, node *& _node, node * lastNode, int origLineNum) const
{
    if (_node) {
        if (origLineNum == lineNum)
            insertSibling(parent, curNode, _node, lastNode);
        else
            insertChild(parent, curNode, _node, lastNode);
    }
}


/***
 * This function inserts a node as a sibling to the CST
 *
 * @param parent current root node of the CST structure
 * @param curNode current place in the structure
 * @param _node child (root node of that set structure) node to be inserted
 * @param lastNode the last node of the child structure
 */
void CST::insertSibling(node *& parent, node *& curNode, node *& _node, node * lastNode)
{
    // ensure the child node is not null
    if (!_node)
        return;

    // CST is empty
    if (!parent)
    {
        parent = _node;
        curNode = lastNode;
    }
    else
    {
        // assert to ensure that the current node is not null
        assert(curNode);
        curNode->sibling(_node);
        curNode = lastNode;
    }
    _node = nullptr;
}


/***
 * This function inserts a node as a child to the CST
 *
 * @param parent current root node of the CST structure
 * @param curNode current place in the structure
 * @param _node child (root node of that set structure) node to be inserted
 * @param lastNode the last node of the child structure
 */
void CST::insertChild(node *& parent, node *& curNode, node *& _node, node * lastNode)
{
    // ensure the node is not null
    if (!_node)
        return;

    // CST is currently empty
    if (!parent)
    {
        parent = _node;
        curNode = lastNode;
    }
    else
    {
        // assert to ensure that the current node is not null
        assert(curNode);
        curNode->child(_node);
        curNode = lastNode;
    }
    _node = nullptr;
}


/***
 * This delete all the nodes in a CST (used in the destructor)
 * @param _node root node of the structure to be deallocated
 */
void CST::deleteNodes(node * _node)
{
    node * temp = nullptr;
    while (_node)
    {
        temp = _node;
        if (_node->sibling())
        {
            _node = _node->sibling();
        }
        else if (_node->child())
        {
            _node = _node->child();
        }
        else
        {
            break;
        }
        delete temp;
    }
    delete _node;
}


/***
 * This gets the last/end node in a CST structure
 *
 * @param root the root node of the structure
 * @return the last node of the structure if
 * the root is not null (otherwise return the itself which is null)
 */
node* CST::getLastNode(node* root)
{
    node* iter = root;
    while (iter) {
        // iterate to the last sibling
        while (iter->sibling())
            iter = iter->sibling();
        // check if it has a child
        if (iter->child()) {
            iter = iter->child();
        } else {

            break;
        }
    }
    return iter;
}


/***
 * This is the main (overhead) parse function that starts
 * the recursive descent
 *
 * @param iter the iterator node for the token list
 * @param outFS the output file stream
 * @return the root node of the CST
 */
node * CST::parse(token *& iter) {

    // if the token list is empty, return immediately
    if (!iter)
        return nullptr;

    // initialize main node pointers
    node * parent = nullptr, *_node = parent, * child = nullptr;
    int origLineNum = lineNum;

    while (iter)
    {
        // reset line change flag variable
        origLineNum = lineNum;
        if (iter->isEndlToken())
        {
            // if there are more nodes after current, enter the parseEndl function
            if (iter->next()) {
                parseEndlToken(iter);
            }
                // manually iterate for simplicity
            else {
                iter = iter->next();
                lineNum++;
            }
            continue;
        }
        // procedure
        if (match(iter, "procedure"))
            child = parseProcedureDeclaration(iter);
            // function
        else if (match(iter, "function"))
            child = parseFunction(iter);
            // global variable
        else if (isDatatype(iter))
            child =  parseDeclarationStatement(iter);
        else
        {
            // this is for an invalid statement in the global scope
            errorDetected = true;
            outFS << "Syntax error on line " << lineNum << ": INVALID STATEMENT";
        }
        // insert the child node and flush the file stream to immediately report errors (if encountered)
        insert(parent, _node, child, getLastNode(child), origLineNum);
        outFS.flush();
        // reset scope to 0
        curScope = 0;
    }
    // return parent node of the created CST structure
    return parent;

}


/***
 * This parses a single quote string
 *
 * @param iter the current node in the token list
 * @param outFS the file stream for errors
 * @return root node of the CST structured string (if formatted correctly)
 */
node * CST::parseSingleQuoteString(token *& iter)
{
    // ensure the current node is not null
    if (!iter)
        return nullptr;

    int origLineNum = lineNum;
    // check if the statement starts with a single quote
    if (!matchType(iter, "SINGLE_QUOTE"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED SINGLE QUOTE\n";
    }

    // create a node for the opening quote
    auto * parent = new node("\'", curScope);
    node * _node = parent;
    iter = iter->next();

    // ensure that the string follows the structure of an empty/non-empty string
    if (!matchType(iter, STRING) && !matchType(iter, STRING) &&
        !matchType(iter, "SINGLE_QUOTE"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED STRING/END QUOTE\n";
    }

    // if the string is not empty, create a node for the inside string and insert it
    if (matchType(iter, STRING)) {
        auto *child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();
    }
    // ensure there is a closing quote for the string
    if (!match(iter, "\'"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED END QUOTE\n";
    }

    // create a node for the ending quote
    auto * child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    // iterate past the end quote
    iter = iter->next();
    return parent;
}


/***
 * This parses past the end line token
 * @param iter the current node in the token list
 */
void CST::parseEndlToken(token *& iter) {

    while (iter && iter->isEndlToken()) {
        if (iter->isEndlToken())
            lineNum++;
        iter = iter->next();
    }

}


/***
 * This parses until to the token right before an end line token,
 * this is used in some error statements
 *
 * @param iter current node in the token list
 */
void CST::parseUntilEndl(token *& iter)
{
    // iterate past all the tokens until the end line
    while (iter && !iter->isEndlToken())
        iter = iter->next();
}


/***
 * This parses an assignment statement
 *
 * @param iter the current node in the token list
 * @param outFS the file stream for errors
 * @return the root node of the assignment statement structure
 */
node * CST::parseAssignmentStatement(token *& iter)
{

    int origLineNum = lineNum;
    // ensure the statement starts with an identifier
    if (!matchType(iter, IDENTIFIER))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected identifier\n";
    }

    // create the parent node with the identifier
    auto * parent = new node(iter->content(), curScope);
    string variableName = iter->content(); // used to reference the symbol table
    node * child, * _node = parent;
    iter = iter->next();

    // check if you are referencing an index
    if (match(iter, "[")) {
        parseBracket(iter, parent, child, _node, variableName, origLineNum);
    }

    // ensure there is an assignment operator in the expression
    if (!match(iter, "="))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected ASSIGNMENT OPERATOR\n";
    }

    // catch the assignment operator
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    // create a child node from the expression after the assigment
    child = parseExpression(iter);

    // insert the child node from the expression
    insert(parent, _node, child, getLastNode(child), origLineNum);

    // ensure the line ends with a semicolon
    if (!match(iter, ";"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected SEMICOLON\n";
    }

    // catch the semicolon
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    // parse until next actual node
    parseEndlToken(iter);

    return parent;

}


/***
 * This function parses an increment statement (used in for loops)
 *
 * @param iter current node in token list
 * @param outFS file stream for errors
 * @return root node of the increment statement
 */
node * CST::parseIncrementStatement(token *& iter)
{
    int origLineNum = lineNum;
    // ensure statement starts with an identifier
    if (!matchType(iter, IDENTIFIER))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected identifier\n";
    }

    // catch the identifier
    auto * parent = new node(iter->content(), curScope);
    node * child, * _node = parent;
    iter = iter->next();

    // check for assigment operator
    if (!match(iter, "="))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected ASSIGNMENT OPERATOR\n";
    }

    // catch assigment operator
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    // catch the expression after the assignment operator
    child = parseExpression(iter);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    return parent;

}


/***
 * This function parses a double quote string
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the DQS
 */
node * CST::parseDoubleQuoteString(token *& iter) {

    int origLineNum = lineNum;
    // ensure the statement starts with a double quote
    if (!match(iter, "\""))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED DOUBLE QUOTE\n";
    }

    // catch the double quote
    auto * parent = new node(iter->content(), curScope);
    node * _node = getLastNode(parent);
    iter = iter->next();

    // ensure that the string follows the structure of empty/non-empty strings
    if (!matchType(iter, STRING) && !matchType(iter, "DOUBLE_QUOTE"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED DOUBLE STRING/END QUOTE\n";
    }

    // if the string is non-empty
    if (matchType(iter, STRING))
    {
        // catch the contained string and insert it
        auto *child = new node(iter->content(), curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();
    }
    // ensure there is a closing quote
    if (!match(iter, "\""))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": NO ENDING DOUBLE QUOTE\n";
    }
    // catch closing quote
    auto * child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();
    return parent;

}


/***
 * This is a helper function that recursively parses statements for the block statement
 * parsing function
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return
 */
node* CST::parseCompoundStatement( token *& iter) {

    int origLineNum = lineNum;
    node* parent = nullptr;
    node* _node = parent;
    node * child = nullptr;

    // iterate while there are statements in the brackets
    while (iter && iter->content()!= "}") {
        // catch the statement and insert it
        child = parseStatement(iter);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        parseEndlToken(iter);
    }
    // return the end structure
    return parent;
}


/***
 * This parses block statements for conditionals, functions, and loops
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the block statement
 */
node* CST::parseBlockStatement( token *& iter) {

    int origLineNum = lineNum;
    // ensure the statement begins with a left bracket
    if (!match(iter, "{"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected '{' to start block\n";
    }

    // catch beginning bracket and initialize other pointers
    node* parent = new node(iter->content(), curScope);
    node* _node = parent;
    node* child = nullptr;
    iter = iter->next();

    // parse to next actual token
    parseEndlToken(iter);

    // parse the statements contained in the brackets
    node* body = parseCompoundStatement(iter);
    insert(parent, _node, body, getLastNode(body), origLineNum);

    // parse to next actual token
    parseEndlToken(iter);

    // ensure there is a closing bracket after the statements
    if (!match(iter, "}"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected '}' to close block\n";
    }

    // catch closer bracket and return
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();
    return parent;
}


/***
 * This is the main statement parsing function that determines which
 * helper function to be called
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return
 */
node* CST::parseStatement(token *& iter) {

    int origLineNum = lineNum;
    // ensure the iterator is not null
    if (!iter)
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Statement expected but pointer is null\n";
    }
    // conditional statement
    else if (match(iter, "if"))
    {
        return parseSelectionStatement(iter);
    }
        // block statement
    else if (match(iter, "{"))
    {
        return parseBlockStatement(iter);
    }
        // iteration statement
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
        // get char statement
    else if (match(iter,  "getChar")) {
        return parseGetCharFunction(iter);
    }
        // declaration statement
    else if (isDatatype(iter)) {
        return parseDeclarationStatement(iter);
    }
        // begins with identifier, either assigment or user function
    else if (matchType(iter, IDENTIFIER)) {
        token * peek = iter->next();
        if (match(peek, "=") || match(peek, "["))
            return parseAssignmentStatement(iter);
        else
            return parseUserDefinedFunctionStatement(iter);
    }
    // current node is an end line token, parse past it and return based off next token
    else
    {
        parseEndlToken(iter);
        return parseStatement(iter);
    }
    return nullptr; // should not be called
}


/***
 * This function parses a printf function call
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the statement
 */
node* CST::parsePrintfStatement( token *& iter) {

    int origLineNum = lineNum;
    // ensure the statement begins with "printf"
    if (!match(iter, "printf"))
    {
        errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected 'printf'\n";
    }

    // catch printf
    node* parent = new node(iter->content(), curScope);
    node* _node = parent;
    node* child = nullptr;
    iter = iter->next();

    // make sure there is a parentheses after printf
    if (!match(iter, "("))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected '(' after 'printf'\n";
    }
    // catch left parentheses
    child = new node("(", curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    // check the type of string
    // double quote string
    if (match(iter, "\"")) {
        child = parseDoubleQuoteString(iter);
    }
        // single quote string
    else if (match(iter, "'")) {
        child = parseSingleQuoteString(iter);
    }
        // current token is not a string
    else {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected a quoted string after printf\n";
    }
    insert(parent,_node, child, getLastNode(child), origLineNum);

    // check if there are variable parameters after the string
    if (match(iter, ","))
    {
        // catch initial comma
        child = new node(iter->content(), curScope);
        insert(parent, _node, child, child, origLineNum);
        iter = iter->next();

        // catch the parameters
        node* args = parseIdentifierAndIdentifierArrayList(iter);
        insert(parent,_node, args, getLastNode(args), origLineNum);
    }

    // check for ending right parentheses
    if (!match(iter, ")"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected ')' after printf arguments\n";
    }
    // catch ending parentheses
    child = new node(")",curScope);
    insert(parent, _node, child, child, origLineNum);
    iter = iter->next();

    // ensure there is an ending semicolon
    if (!match(iter, ";"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected ';' after printf statement\n";
    }
    // catch semicolon
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, child, origLineNum);
    iter = iter->next();
    return parent;
}


node * CST::parseBracket( token *& iter, node *& parent, node *& child, node *& _node,
                          const string& variableName, const int& origLineNum)
{
    stNode * symbolNode = ST->retrieveNode(variableName, curScope);
    // check if a non-array variable is being referenced with an index
    if (symbolNode && !symbolNode->isArr())
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Attempted to reference an index of a non-array variable\n";
        delete parent;
        parseUntilEndl(iter); // parse to the end line
        parseEndlToken(iter); // parse until actual token
        return nullptr;
    }
    // catch the initial bracket
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, child, origLineNum);

    iter = iter->next();

    // ensure that the size variable is an integer
    if (!matchType(iter, INTEGER) && !(matchType(iter, IDENTIFIER)))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected array index as integer\n";
    }
    else
    {
        // check if the current token is a negative integer if it is an integer
        if (matchType(iter, INTEGER) && iter->content()[0] == '-')
        {
            errorDetected = true;
            outFS << "Syntax error on line " << lineNum << ": Expected array index to be a positive integer\n";
        }
    }

    // create a node for the size variable/integer
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, child, origLineNum);
    iter = iter->next();

    // ensure there is a closing bracket
    if (!match(iter, "]"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected closing ']' for array declaration\n";
    }

    // catch closing bracket
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, child, origLineNum);
    iter = iter->next();
    return parent;
}


/***
 * This function parses a list of identifiers for function calls, printf statements, etc
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the list
 */
node* CST::parseIdentifierAndIdentifierArrayList( token *& iter) {

    int origLineNum = lineNum;
    // ensure the statement begins with an identifier
    if (!matchType(iter, IDENTIFIER))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected identifier\n";
    }

    // catch initial identifier
    node* parent = new node(iter->content(), curScope);
    string variableName = iter->content();
    node* _node = getLastNode(parent);
    node* child;
    iter = iter->next();

    // ensure variable exists in the scope
    if (!ST->exists(variableName, curScope))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": unknown variable referenced\n";
    }

    while (iter) {

        // current parameter is an array
        if (match(iter, "[")) {
            parseBracket(iter, parent, child, _node, variableName, origLineNum);
        }

        // check if there are additional parameters
        if (match(iter, ","))
        {
            // catch comma
            child = new node(iter->content(), curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();

            // ensure there is an identifier after the comma
            if (!matchType(iter, IDENTIFIER))
            {
                errorDetected = true;
                outFS << "Syntax error on line " << lineNum << ": NO IDENTIFIER AFTER COMMA\n";
            }

            // catch the identifier
            child = new node(iter->content(), curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();

        }
        else
        {
            break;
        }
    }

    return parent;
}


/***
 * This function parses an iteration statement (for/while loop)
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the iteration statement
 */
node* CST::parseIterationStatement( token *& iter) {

    int origLineNum = lineNum;
    // ensure the pointer is not null
    if (!iter)
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Unexpected end of input in iteration statement\n";
    }

    // initialize node pointers
    node* parent = nullptr;
    node* _node = parent;
    node* child = nullptr;

    // check if it is a while loop
    if (match(iter, "while"))
    {
        // catch the while
        parent = new node("while", curScope);
        _node = parent;
        iter = iter->next();

        // ensure there is a parentheses after while
        if (!match(iter, "("))
        {
            errorDetected = true;
            outFS << "Syntax error on line " << lineNum << ": Expected '(' after 'while'\n";
        }

        // catch left parentheses
        child = new node("(", curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();

        // catch the condition
        node* condition = parseBooleanExpression(iter);
        insert(parent,_node, condition, getLastNode(condition), origLineNum);

        if (!match(iter, ")"))
        {
            errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected ')' after while condition\n";}

        // catch ending parentheses
        child = new node(")", curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();

        // reset line flag variable and parse to next token
        origLineNum = lineNum;
        parseEndlToken(iter);

        // insert the statements in the loop and return
        node* stmt = parseStatement(iter);
        insert(parent,_node, stmt, getLastNode(stmt), origLineNum);
        return parent;
    }
        // check for "for" loop
    else if (match(iter, "for"))
    {
        // catch for
        parent = new node("for", curScope);
        _node = parent;
        iter = iter->next();

        // ensure there is a left parentheses after for
        if (!match(iter, "("))
        {
            errorDetected = true;
            outFS << "Syntax error on line " << lineNum << ": Expected '(' after 'for'\n";
        }

        // catch left parentheses
        child = new node("(", curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();

        // catch initialization expression
        node* init = parseAssignmentStatement(iter);
        insert(parent, _node, init, getLastNode(init), origLineNum);

        // catch the condition
        node* condition = parseBooleanExpression(iter);
        insert(parent,_node, condition, getLastNode(condition),origLineNum);

        // ensure there is a semicolon after
        if (!match(iter, ";"))
        {
            errorDetected = true;
            outFS << "Syntax error on line " << lineNum << ": Expected second ';' in for-loop\n";
        }
        // catch semicolon
        child = new node(";", curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();

        // catch increment statement
        node* update = parseIncrementStatement(iter);
        insert(parent,_node, update, getLastNode(update),origLineNum);

        // check for ending parentheses
        if (!match(iter, ")"))
        {
            errorDetected = true;
            outFS << "Syntax error on line " << lineNum << ": Expected ')' after for-loop header\n";
        }
        // catch ending parentheses
        child = new node(")", curScope);
        insert(parent, _node, child, getLastNode(child), origLineNum);
        iter = iter->next();

        // consume and insert statements within the bounds of the for loop
        node* stmt = parseStatement(iter);
        insert(parent,_node, stmt, getLastNode(stmt), origLineNum);
        return parent;
    }
        // invalid iteration statement
    else
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected 'for' or 'while' at start of iteration statement\n";
    }
    return nullptr;
}


/***
 * This function parses a return statement
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the return statement
 */
node* CST::parseReturnStatement( token *& iter)
{
    int origLineNum = lineNum;
    // ensure the statement begins with return
    if (!match(iter, "return"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected 'return'\n";
    }
    // catch return
    node* parent = new node(iter->content(), curScope);
    node* _node = parent, *child;
    iter = iter->next();

    // parse expression
    child = parseExpression(iter);
    insert(parent, _node, child, getLastNode(child), origLineNum);

    // ensure there is a semicolon after the expression
    if (!match(iter, ";"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected ';' at end of return statement\n";
    }

    // catch the semicolon
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();
    return parent;
}


/***
 * This function parses a declaration statement
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the declaration statement
 */
node * CST::parseDeclarationStatement(token *& iter)
{
    // symbol table parameters
    string idName, idType = "datatype", datatype;
    bool isArr = false;
    int arrSize = 0;

    int origLineNum = lineNum;
    // ensure the statement begins with a datatype
    if (!isDatatype(iter))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected datatype\n";
    }

    // initialize node pointers and catch datatype
    auto *parent = new node(iter->content(), curScope);
    datatype = iter->content();
    node *_node = parent, *child = nullptr;
    iter = iter->next();


    while (!match(iter, ";"))
    {
        if (!matchType(iter, IDENTIFIER)) {
            { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected identifier in declaration\n";}
        }

        idName = iter->content();
        node *idNode = new node(iter->content(), curScope);
        insert(parent,_node, idNode, getLastNode(idNode),origLineNum);
        iter = iter->next();

        // check if variable is an array
        if (match(iter, "[")) {

            isArr = true;
            parseBracket(iter, parent, child, _node, idName, origLineNum);
        }

        auto * variable = new stNode(idName, idType, datatype, isArr, arrSize, curScope);
        int foundScope;
        // ensure the variable name is not already used
        if (ST->inUse(variable, foundScope))
        {
            outFS << "Error on line " << lineNum << ": variable \"" << idName << "\" is already defined ";
            if (foundScope == 0)
                outFS << "globally\n";
            else
                outFS << "locally\n";
            errorDetected = true;
            delete variable;
        }
            // variable name is also not a reserve word
        else if (checkReserveWord(variable->idName(), VAR_RESERVED_WORDS))
        {
            outFS << "Syntax Error on line " << lineNum << ": the name " << idName << " cannot be used as it";
            outFS << " is a reserve word\n";
            errorDetected = true;
            delete variable;
        }
            // insert it to the symbol table
        else
            ST->insert(variable);

        // check if there are additional declarations
        if (match(iter, ","))
        {
            // catch comma
            child = new node(iter->content(), curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();

            // reset symbol table parameters
            isArr = false;
            arrSize = 0;
        }


    }
    // ensure there is a semicolon after the declaration(s)
    if (!match(iter, ";"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected semicolon to end declaration statement\n";
    }
    // catch semicolon
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();
    return parent;
}


/***
 * This function parses a selection (if) statement
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the selection statement
 */
node* CST::parseSelectionStatement( token *& iter)
{
    int origLineNum = lineNum;
    // ensure statment begins with if
    if (!match(iter, "if"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected 'if' at start of selection statement\n";
    }

    // catch if and initialize other node pointers
    auto* parent = new node(iter->content(), curScope);
    auto* _node = parent;
    node * child = nullptr;
    iter = iter->next();

    // catch condition
    node* condition = parseBooleanExpression(iter);
    insert(parent,_node, condition, getLastNode(condition),origLineNum);
    origLineNum = lineNum;
    parseEndlToken(iter); // parse to next actual token

    // catch all the statements within the if statement and insert them
    node* stmt = parseStatement(iter);
    insert(parent,_node, stmt, getLastNode(stmt), origLineNum);

    // update line flag variable and parse to next token
    origLineNum = lineNum;
    parseEndlToken(iter);

    // check if there is an else
    if (match(iter, "else")) {

        // catch else
        child = new node(iter->content(), curScope);
        insert(parent, _node, child, child, origLineNum);
        iter = iter->next();

        // reset flag
        origLineNum = lineNum;
        parseEndlToken(iter);

        // catch all the statements within the else statement and insert them
        stmt = parseStatement(iter);
        insert(parent,_node, stmt, getLastNode(stmt), origLineNum);
    }

    return parent;
}


/***
 * This function parses a procedure
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of procedure structure
 */
node * CST::parseProcedureDeclaration(token *& iter)
{
    // symbol table parameters
    string idName, idType = "procedure", datatype = "NOT APPLICABLE";
    prevScope++;
    curScope = prevScope;

    int origLineNum = lineNum;
    // ensure initial statement begins with procedure
    if (!match(iter, "procedure"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED PROCEDURE\n";
    }

    // catch procedure
    auto * parent = new node(iter->content(), curScope);
    node * _node = getLastNode(parent);
    iter = iter->next();

    // ensure the name of the procedure is an identifier
    if (!matchType(iter, IDENTIFIER))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED IDENTIFIER\n";
    }

    // set current function and id name to the name of the procedure
    curFunction = idName = iter->content();

    // check if name is a reserve word
    if (checkReserveWord(idName, FUNCTION_SPECIFIC_RESERVES))
    {
        outFS << "Syntax error on line " << lineNum << ": " << idName << " can not be used as the name of a procedure\n";
        errorDetected = true;
    }

    // catch the function name
    auto * child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    // create a symbol table node and insert it into the symbol table
    auto * procedure = new stNode(idName, idType, datatype, curScope);
    ST->insert(procedure);

    // ensure the procedure has a left parentheses
    if (!match(iter, "("))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED (\n";
    }
    // catch left parentheses
    child = new node("(", curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    // catch parameter list (if it exists)
    auto * paramList = parseParameterList(iter);
    // if parameter list is null, ensure it has void instead
    if (!paramList && !match(iter, "void") )
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED void\n";
    }

    symbolTable * PL = nullptr;
    // if there is a parameter list, insert it into the symbol table
    if (paramList) {
        child = paramList;
        PL = createPL(paramList);
        ST->insertST(PL);
    }
    else
        child = new node(iter->content(), curScope);

    // insert parameter list/void
    insert(parent, _node, child, getLastNode(child), origLineNum);
    if (!paramList)
        iter = iter->next();

    // ensure there is a closing parentheses
    if (!match(iter, ")"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED )\n";
    }

    // catch ending parentheses
    child = new node(")", curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    // reset line flag
    origLineNum = lineNum;
    parseEndlToken(iter);

    // insert the block statement and return
    child = parseBlockStatement(iter);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    return parent;

}


/***
 * This function parses a getChar statement
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the getChar statement
 */
node* CST::parseGetCharFunction( token *& iter)
{
    int origLineNum = lineNum;
    // ensure the statement begins with getChar
    if (!match(iter, "getchar"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected 'getchar'\n";
    }

    // catch getChar and initialize other node pointers
    node* parent = new node(iter->content(), curScope);
    node* _node = getLastNode(parent);
    node* child;
    iter = iter->next();

    // check for left parentheses
    if (!match(iter, "("))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected '(' after 'getchar'\n";
    }

    // catch left parentheses
    child = new node("(", curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    // check for ending parentheses
    if (!match(iter, ")"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected ')' after 'getchar()'\n";
    }

    // catch ending parentheses
    child = new node(")", curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();
    return parent;
}


/***
 * This checks if a given node is a boolean operator
 * @param iter current node in the token list
 * @return true if iter contains a boolean operator, false if not
 */
bool CST::isBooleanOperator(token *&iter) {

    return (iter && (iter->content() == "<=" || iter->content() == ">=" || iter->content() == "!" ||
                     iter->content() == "==" || iter->content() == "<" || iter->content()== ">" ||
                     iter->content() == "&&" || iter->content() == "!=" || iter->content()== "||"));

}


/***
 * This parses a parameter list
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the parameter list
 */
node * CST::parseParameterList(token *& iter)
{
    int origLineNum = lineNum;
    // ensure the list begins with a parameter
    if (!isDatatype(iter)) {
        return nullptr;
    }

    // catch initial datatype
    auto * parent = new node(iter->content(), curScope);
    auto * _node = parent;
    iter = iter->next();

    // ensure current token is an identifier
    if (!matchType(iter, IDENTIFIER))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED IDENTIFIER\n";
    }

    // catch identifier
    auto * child = new node(iter->content(), curScope);
    string variableName = iter->content();
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    // check if variable is an array
    if (match(iter, "[")) {
        parseBracket(iter, parent, child, _node, variableName, origLineNum);
    }

    // check for additional parameters and insert them recursively
    if (match(iter, ","))
    {
        child = new node(iter->content(), curScope);
        insert(parent, _node, child, child, origLineNum);
        iter = iter->next();
        child = parseParameterList(iter);
        insert(parent, _node, child, getLastNode(child), origLineNum);


    }
    return parent;

}


/***
 * This checks if a given node holds a datatype
 * @param iter current node in token list
 * @return if the node holds a datatype, false if not
 */
bool CST::isDatatype(token *& iter)
{
    if (!match(iter, "char") && !match(iter, "bool")
        && !match(iter, "int"))
        return false;
    return true;
}


/***
 * This parses a boolean expression
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the expression
 */
node* CST::parseBooleanExpression( token *& iter) {

    // initialize pointers and line flag variable
    node* parent = nullptr;
    node * _node = parent, * child = nullptr;
    int origLineNum = lineNum;

    // check cases for non-boolean operators
    if (!isBooleanOperator(iter))
    {
        // expression is contained within parentheses
        if (match(iter, "(")) {

            // catch initial parentheses
            parent = new node("(", curScope);
            _node = parent;
            iter = iter->next();

            // catch expression
            child = parseBooleanExpression(iter);
            insert(parent, _node, child, getLastNode(child), origLineNum);

            // ensure there is an ending parentheses
            if (!match(iter, ")"))
            {
                errorDetected = true;
                outFS << "Syntax error on line " << lineNum << ": MISSING CLOSING PARENTHESES\n";
            }

            // catch ending parentheses
            child = new node(")", curScope);
            insert(parent, _node, child, getLastNode(child), origLineNum);
            iter = iter->next();
        }
            // check if condition is entirely a false or true statement
        else if (match(iter, "TRUE") || match(iter, "FALSE"))
        {
            parent = new node(iter->content(), curScope);
            iter = iter->next();
        }
            // single quote string
        else if (match(iter, "'")) {
            parent = parseSingleQuoteString(iter);

        }
            // double quote string
        else if (match(iter, "\"")) {
            parent = parseDoubleQuoteString(iter);
        }
            // numerical expression
        else {
            parent = parseNumericalExpression(iter);
        }
        _node = getLastNode(parent);
    }

    // iterate through the rest of the expression
    while (isBooleanOperator(iter))
    {
        // catch the operator
        node* op = new node(iter->content(), curScope);
        insert(parent,_node, op, getLastNode(op),origLineNum);
        iter = iter->next();

        // catch the rest of the expression/next term
        node* right = parseBooleanExpression(iter);
        insert(parent,_node, right, getLastNode(right),origLineNum);
    }

    return parent;
}


/***
 * This creates a parameters list for the symbol table
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return the created symbol table
 */
symbolTable * CST::createPL(node * iter)
{
    // initialize base parameters
    auto * PL = new symbolTable(curFunction);
    string idName, idType = "datatype", datatype;
    bool isArr = false;
    int size = 0;

    while (iter)
    {
        // get the datatype
        datatype = iter->content();
        iter = iter->sibling();

        // get the name
        idName = iter->content();
        iter = iter->sibling();

        // check if array
        if (iter && iter->content() == "[")
        {
            isArr = true;
            iter = iter->sibling();

            // check if a size is given
            if (iter->content() != "]")
            {
                size = stoi(iter->content());
                iter = iter->sibling();
            }
            // iterate past the end bracket
            iter = iter->sibling();

        }
        // check if not end of list
        if (iter && iter->content() == ",")
            iter = iter->sibling();

        // create a new stNode with populated parameters
        auto * variable = new stNode(idName, idType, datatype, isArr, size, curScope);
        int foundScope;
        // check if variable is in use
        if (ST->inUse(variable, foundScope))
        {
            outFS << "Error on line " << lineNum << ": variable \"" << idName << "\" is already defined ";
            if (foundScope == 0)
                outFS << "globally\n";
            else
                outFS << "locally\n";
            errorDetected = true;
            delete variable;
        }
            // check if variable is a reserve word
        else if (checkReserveWord(variable->idName(), VAR_RESERVED_WORDS))
        {
            outFS << "Syntax Error on line " << lineNum << ": the name " << idName << " cannot be used as it\n";
            outFS << " is a reserve word\n";
            errorDetected = true;
            delete variable;
        }
            // insert to the parameter list
        else
            PL->insert(variable);
        outFS.flush();

        // reset the parameters (for the symbol table)
        datatype.clear();
        idName.clear();
        size = 0;
        isArr = false;
    }
    return PL;

}


/***
 * This parses a function
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the function
 */
node * CST::parseFunction(token *& iter)
{
    // symbol table parameters
    string idName, idType = "function", datatype;
    prevScope++;
    curScope = prevScope;

    int origLineNum = lineNum;
    // ensure statement begins with function
    if (!match(iter, "function"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED FUNCTION\n";
    }

    // catch function
    auto * parent = new node(iter->content(), curScope);
    node * _node = getLastNode(parent);
    iter = iter->next();

    // check return type
    if (!isDatatype(iter))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED DATATYPE\n";
    }

    // catch return type
    datatype = iter->content();
    auto * child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    // ensure function name is an identifier
    if (!matchType(iter, IDENTIFIER))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED IDENTIFIER\n";
    }

    // set current function and symbol parameter
    curFunction = idName = iter->content();

    // check if name is a reserve word
    if (checkReserveWord(idName, FUNCTION_SPECIFIC_RESERVES))
    {
        outFS << "Syntax error on line " << lineNum << ": " << idName << " can not be used as the name of a function\n";
        errorDetected = true;
    }

    // catch function name
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);

    // create a new symbol table node and insert it
    auto * function = new stNode(idName, idType, datatype, curScope);
    ST->insert(function);
    iter = iter->next();

    // ensure there is a left parentheses after name
    if (!match(iter, "("))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED (\n";
    }

    // catch the parentheses
    child = new node("(", curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    // retrieve the parameter list (if it exists)
    auto * paramList = parseParameterList(iter);
    // if no expected parameters, ensure it has void instead
    if (!paramList && !match(iter, "void") )
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED void\n";
    }

    // populate a new symbol table with the parameter list and insert it
    symbolTable * PL = nullptr;
    if (paramList) {
        child = paramList;
        PL = createPL(child);
        ST->insertST(PL);
    }
    else
        child = new node(iter->content(), curScope);

    insert(parent, _node, child, getLastNode(child), origLineNum);

    // ensure there is a closing parentheses
    if (!match(iter, ")"))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": EXPECTED )\n";
    }

    // catch end parentheses
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    // parse until next actual token
    parseEndlToken(iter);

    // consume the function's statements and insert them into the structure
    child = parseBlockStatement(iter);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    return parent;

}


/***
 * This checks if a given token has a set string
 * @param iter given token
 * @param expected the expected string
 * @return true if the token has the string, false if the token is null
 * or has a different string
 */
bool CST::match(token *& iter, const string& expected)
{
    if (iter && iter->content() == expected)
    {
        return true;
    }
    return false;
}


/***
 * This parses a factor in an expression
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the factor
 */
node* CST::parseFactor( token *& iter)
{
    // initialize node pointers
    int origLineNum = lineNum;
    node* parent = nullptr, *child = nullptr, *_node = nullptr;
    token * peek = iter->next();

    // check if the factor is a parenthesized expression
    if (match(iter, "(")) {

        // catch opening parentheses
        parent = new node("(", curScope);
        _node = parent;
        iter = iter->next();

        // catch expression
        child = parseNumericalExpression(iter);
        insert(parent, _node, child, getLastNode(child), origLineNum);

        // ensure there is an end parentheses
        if (!match(iter, ")"))
        {
            errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected ')'\n";
        }

        // catch closing parentheses
        child = new node(iter->content(), curScope);
        insert(parent, _node, child, child, origLineNum);
        iter = iter->next();
    }
        // check if the token is an identifier or an integer
    else if (matchType(iter, INTEGER) || matchType(iter, IDENTIFIER))
    {
        // if current token is an identifier and next is a parentheses (user function)
        if (matchType(iter, IDENTIFIER) && peek && match(peek, "("))
        {
            return parseUserDefinedFunction(iter);

        }
            // not a user function
        else {
            // catch identifier
            string variableName = iter->content(), prevType = iter->_type();
            parent = new node(iter->content(), curScope);
            _node = parent;
            iter = iter->next();

            // if identifier is an array
            if (prevType == IDENTIFIER && match(iter, "[")) {
                parseBracket(iter, parent, child, _node, variableName, origLineNum);
            }
        }

    }
        // single quote string
    else if (match(iter, "'"))
    {
        parent = parseSingleQuoteString(iter);
    }
        // double quote string
    else if (match(iter, "\""))
    {
        parent = parseDoubleQuoteString(iter);
    }
        // invalid factor
    else
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected identifier, integer, or '('\n";
    }

    return parent;
}


/***
 * This parses a user function
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the user function call
 */
node* CST::parseUserDefinedFunction( token *& iter)
{
    int origLineNum = lineNum;
    // ensure beginning term is an identifier
    if (!matchType(iter, IDENTIFIER))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected function name\n";
    }

    // catch function name
    node* parent = new node(iter->content(), curScope);
    node* _node = parent;
    node* child = nullptr;
    iter = iter->next();

    // ensure there is a left parentheses immediately after
    if (!match(iter, "("))
    {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected '(' after function name\n";
    }

    // catch parentheses
    child = new node("(", curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();

    // if term is an identifier
    if (matchType(iter, IDENTIFIER))
    {

        token* lookahead = iter->next();
        // if the current token is an array, parse past the brackets to check the type
        // of expression is after it
        if (match(lookahead, "["))
        {
            // iterate until you get to the closing bracket
            while (lookahead && !match(lookahead, "]"))
                lookahead = lookahead->next();
            // ensure you do not accidentally reference a null pointer (error is caught in later functions)
            // and iterate past the end bracket
            if (match(lookahead, "]"))
                lookahead = lookahead->next();
        }
        // check for additional parameters
        if (match(lookahead, ","))
        {
            node* args = parseIdentifierAndIdentifierArrayList(iter);
            insert(parent,_node, args, getLastNode(args),origLineNum);
        }
            // must be an expression
        else
        {
            node* expr = parseExpression(iter);
            insert(parent,_node, expr, getLastNode(expr),origLineNum);
        }
    }
        // parse the expression inside the function call
    else
    {

        node* expr = parseExpression(iter);
        insert(parent,_node, expr, getLastNode(expr),origLineNum);
    }

    // ensure there is an ending parentheses
    if (!match(iter, ")"))
    { errorDetected = true; outFS << "Syntax error on line " << lineNum << ": Expected ')' after function arguments\n";}

    // catch the ending parentheses
    child = new node(iter->content(), curScope);
    insert(parent, _node, child, getLastNode(child), origLineNum);
    iter = iter->next();
    return parent;
}


/***
 * This is use d if a user function call is an entire statement itself
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the user function call
 */
node* CST::parseUserDefinedFunctionStatement( token *& iter)
{
    int origLineNum = lineNum;
    // call helper
    auto *parent = parseUserDefinedFunction(iter), *_node = getLastNode(parent);
    // ensure there is an ending semicolon
    if (!match(iter, ";")) {
        errorDetected = true;
        outFS << "Syntax error on line " << lineNum << ": Expected semicolon after user function\n";
    }
    // catch ending semicolon
    auto *child = new node(";", curScope);
    insert(parent, _node, child, child, origLineNum);
    iter = iter->next(); // parse past semicolon
    return parent;
}


/***
 * This function parses an expression
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the expression
 */
node* CST::parseExpression( token *& iter) {

    token* peek = iter;

    // check if expression is a boolean expression
    while (peek && !peek->isEndlToken())
    {
        // check if current peek token is a boolean operator
        if (isBooleanOperator(peek)) {
            return parseBooleanExpression(iter);
        }

        if (peek->content() == ";" || peek->content()  == ")" || peek->content()  == ",")
            break;

        peek = peek->next();
    }
    // numerical expression
    return parseNumericalExpression(iter);
}


/***
 * This function parses a numerical expression
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the expression
 */
node* CST::parseNumericalExpression( token *& iter)
{
    int origLineNum = lineNum;
    // parse the initial term
    node* parent = parseTerm(iter);
    node* _node = getLastNode(parent);

    // while there are addition/subtractions
    while (match(iter, "+") || match(iter, "-"))
    {
        // catch operator
        node* op = new node(iter->content(), curScope);
        insert(parent,_node, op, getLastNode(op), origLineNum);
        iter = iter->next();

        // parse next term
        node* right = parseTerm(iter);
        insert(parent,_node, right, getLastNode(right), origLineNum);
    }

    return parent;
}


/***
 * This parses a term for expressions
 *
 * @param iter current node in the token list
 * @param outFS file stream for errors
 * @return root node of the expression
 */
node* CST::parseTerm( token *& iter)
{
    int origLineNum = lineNum;
    // catch the initial factor
    node* parent = parseFactor(iter);
    node * _node = getLastNode(parent);

    // while there are high precedence operators
    while (match(iter, "*") || match(iter, "/") || match(iter, "%"))
    {
        // catch operator
        node* op = new node(iter->content(), curScope);
        insert(parent,_node, op, getLastNode(op), origLineNum);
        iter = iter->next();

        // catch next factor
        node* right = parseFactor(iter);
        insert(parent,_node, right, getLastNode(right), origLineNum);
    }

    return parent;
}


/***
 * This checks if a given token is a set type
 *
 * @param iter given token
 * @param expected type of token to be compared
 * @return true if the token is that type, false if not
 */
bool CST::matchType(token *& iter, const string& expected)
{
    if (iter && iter->_type() == expected)
    {
        return true;
    }
    return false;
}


/***
 * This outputs the contents of a CST in a human-readable way
 * @param outputFile file to hold the contents of the CST
 */
void CST::outputTraversal(const string& outputFile)
{
    outFS.open(outputFile);

    if (!outFS.is_open())
    {
        cout << "ERROR OPENING " << outputFile << endl;
        exit(3);
    }

    traverseCST(head, outFS);
//    if (head)
//        outFS << "null";

    outFS.close();

}

#include <iomanip>
#define ORIGINAL_LENGTH 1
int lineWidth = ORIGINAL_LENGTH;


/***
 * This traverse the CST and outputs the tokens in their tree
 * structure
 * @param head root node of the tree
 * @param outFS output file stream
 */
void traverseCST(node * head, ofstream& outFS)
{
    if (!head)
        return;
//    outFS << "[" << head->content() << "]->";
//    if (head->sibling())
//        lineWidth += head->content().size() + 4;
    outFS << head->content() << " ";
//    if (head->sibling())
//        lineWidth += head->content().size();

    traverseCST(head->sibling(), outFS);

    if (head->child())
    {
        outFS << "\n\n" << setw(lineWidth);
        traverseCST(head->child(), outFS);


        lineWidth = ORIGINAL_LENGTH;

    }

}


                                                                                                                   
