//
// Created by natem on 4/22/2025.
//

#include "interpreter.h"
#include <stack>


/***
 * This is the main constructor of the interpreter object
 *
 * @param _ast the given AST
 * @param outputFile output file to hold the output from the input file
 */
interpreter::interpreter(AST * _ast, const string& outputFile)
{
   ast = _ast;
   outFS.open(outputFile);

   if (!outFS.is_open())
        throw runtime_error("Failed to open output file\n");

   auto * mainNode = ast->main();
   if (!mainNode)
       throw runtime_error("Program has no main !\n");

   parseFunction(mainNode);
   outFS.close();
}


/***
 * This checks if a given ASTnode holds a boolean operator
 * @param iter the given ASTnode
 * @return true if its content is a boolean operator, false if not
 */
bool interpreter::isBooleanOp(ASTnode * iter)
{
    return (iter && (iter->content() == "!" || iter->content() == "<=" || iter->content() == ">=" || iter->content() == "!" ||
                                iter->content() == "==" || iter->content() == "<" || iter->content()== ">" ||
                                iter->content() == "&&" || iter->content() == "!=" || iter->content()== "||"));
}


/***
 * This parses a given function
 *
 * @param root the first node of the function
 * @return the return value of the function (empty if procedure)
 */
string interpreter::parseFunction(ASTnode *&root)
{
    if (root)
    {
        while (root && root->content() != BEGIN_BLOCK)
            nextNode(root);
        return parseBlockStatement(root);
    }
    return "";
}


/***
 * This parses a return statement
 *
 * @param root first node of the expression
 * @return a converted (to string) value of the evaluated
 * expression
 */
string interpreter::parseReturnStatement(ASTnode *& root)
{
    if (root)
    {
        root = root->sibling(); // parse past return
        return evaluateExpression(root);

    }
    return "";
}


/***
 * This parses past a given block statement, this is
 * typically used to correctly iterate past conditionals and loop
 * @param root the given node in the AST
 */
void interpreter::goPastBlockStatement(ASTnode *& root)
{
    while (root && root->content() != BEGIN_BLOCK)
        nextNode(root);
    nextNode(root);
    int bracketCtr = 1;

    while (root && bracketCtr != 0)
    {
        if (root->content() == BEGIN_BLOCK)
            bracketCtr++;
        else if (root->content() == END_BLOCK)
            bracketCtr--;
        nextNode(root);
    }

}


/***
 * This parses a block statement
 *
 * @param root first node of the block statement (BEGIN_BLOCK)
 * @return the return value of the block statement if it has one,
 * otherwise it returns an empty string
 */
string interpreter::parseBlockStatement(ASTnode *& root)
{
    nextNode(root); // parse past initial block
    string result = parseCompoundStatement(root);
    nextNode(root); // parse end block
    return result;
}


/***
 * This parses a compound statement
 *
 * @param root first node of the compound statement
 * @return the return value of the compound statement if it has one,
 * otherwise it returns an empty string
 */
string interpreter::parseCompoundStatement(ASTnode *&root)
{
    string result;
    while (root && root->content() != "END_BLOCK")
    {
        if (root->content() != DECLARATION)
            result = parseStatement(root);
        else
            nextNode(root);
    }
    return result;
}


/***
 * This parses an assignment
 * @param root the ASSIGNMENT block node
 */
void interpreter::parseAssignment(ASTnode *&root)
{
    nextNode(root); // parse assignment block
    evaluateExpression(root);
    nextNode(root); // parse =

}


/***
 * This parses a statement
 *
 * @param root first node of the statement
 * @return the return value (in a string) if it is
 * a return statement, and an empty string if not
 */
string interpreter::parseStatement(ASTnode *&root)
{
    string result;
    if (root->content() == ASSIGNMENT)
    {
        parseAssignment(root);
    }
    else if (root->content() == BEGIN_BLOCK)
    {
        result = parseBlockStatement(root);
    }
    else if (root->content() == "if")
    {
        result = parseConditional(root);
    }
    else if (root->content() == "while")
    {
        result = parseWhileLoop(root);
    }
    else if (root->content() == "FOR EXPRESSION 1")
    {
        result = parseForLoop(root);
    }
    else if (root->content() == "RETURN")
    {
        return parseReturnStatement(root);
    }
    else if (root->content() == "printf")
    {
        parsePrintf(root);
    }
    else if (root->content() == "call")
    {
        ASTnode * funcPtr = root->sibling();
        result = parseCall(funcPtr);
        parseToNextLine(root);
    }
    else
    {
        auto * funcNode = ast->retrieveFunctionPtr(root->content());
        result = parseCall(funcNode);
    }

    return result;

}


/***
 * This parses a conditional statement
 *
 * @param root first node of the conditional statement
 * @return the return value (in a string) if it has
 * a return statement, and an empty string if not
 */
string interpreter::parseConditional(ASTnode *&root)
{
    if (root)
    {
        // check if statement is an if or else
        string statement = root->content(), result;
        nextNode(root); // parse if/else
        if (statement == "if")
        {
            result = evaluateExpression(root); // parse condition
            // if the condition is true
            if (stoi(result) == 1) {
                result = parseBlockStatement(root);
            }
            // iterate past the block statement
            else
                goPastBlockStatement(root);

            // check if an else statement exists
            if (root && root->content() == "else")
            {
                if (result == "0")
                    result = parseConditional(root);
                else
                    goPastBlockStatement(root);
            }

        }
        // statement is an else (this is only called after parsing an if)
        else
            parseBlockStatement(root);
        return result;
    }
    return "";
}


/***
 * This parses a parameter list
 *
 * @param iter the first node of the statement (name of the function)
 * @param function the name of the function
 */
void interpreter::populateParamList(ASTnode *& iter, const string& function)
{
    if (iter)
    {
        int varNum = 0;
        iter = iter->sibling(); // iterate to (
        iter = iter->sibling(); // iterate to first param
        string varName;
        int idx;
        while (iter && iter->content() != ")")
        {
            if (iter->sibling() && iter->sibling()->content() == "[")
            {
                auto * _node = iter->node(); // catch variable node
                iter = iter->sibling(); // iterate to [
                iter = iter->sibling(); // iterate to index
                if (isNumber(iter->content()))
                    idx = stoi(iter->content());
                else
                {
                    idx = stoi(iter->node()->value);
                }
                string converted = {_node->value[idx]};
                iter = iter->sibling(); // iterate to ]
                ast->_ST()->assignCallParam(function, converted, varNum);
                varNum++;
            }
            // check for additional parameters
            else if (iter->content() != ",")
            {
                if (iter->node())
                    ast->_ST()->assignCallParam(function, iter->node()->value, varNum);
                else
                    ast->_ST()->assignCallParam(function, iter->content(), varNum);
                varNum++; // increment idx
            }
            iter = iter->sibling();
        }
        nextNode(iter); // iterate past )
    }
}


/***
 * This checks determines what type of escape character (if any) a string contains
 * @param next the next character
 * @param CL the current string
 * @param idx the current index in the string
 * @param str the string to be outputted
 * @return true if there is an escape char and false if not
 */
bool interpreter::escapeCharacter(const char& next, const string& CL, int idx, string& str)
{
    if (next == 'n')
        str = "\n";
    else if (next == 't')
        str = "\t";
    else if (next == 'x')
    {
        if (idx + 2 < CL.size() && CL[idx + 2] == '0')
            str = "\\x0";
        outFS << " ";
    }
    if (str != "\\x0")
        outFS << str;
    return !str.empty();

}


/***
 * This parses a printf statement
 * @param root the beginning of the printf statement (printf)
 */
void interpreter::parsePrintf(ASTnode *&root)
{
    if (root)
    {
        vector<int> indexesForVariables;
        vector<ASTnode*> variablesToAdd;
        root = root->sibling(); // parse printf to string
        // parse string
        string str = root->content(), escape;
        if (root->sibling())
        {
            root = root->sibling();
            ASTnode * prev = root;
            while (root)
            {
                variablesToAdd.push_back(root);
                prev = root;
                root = root->sibling();
            }
            root = prev;
            parseToNextLine(root);
        }
        else {
            nextNode(root);
        }
        outputPrintf(str, variablesToAdd);
    }

}


/***
 * This is a helper function that parsePrintf uses to output given
 * text to a file
 *
 * @param str the string variable to hold converted escape characters
 * @param variablesToAdd the list of variables to be added to the string
 * (can be empty)
 */
void interpreter::outputPrintf(string& str, const vector<ASTnode*>& variablesToAdd)
{
    int varIdx = 0;
    string escape;
    unsigned int size = str.size();
    for (int i = 0; i < size; i++)
    {
        // if you are currently at an escape character
        if (str[i] == '\\' && i + 1 < size && escapeCharacter(str[i + 1], str, i, escape))
        {
            i += escape.size();
            escape.clear();
        }
        // variable placement
        else if (str[i] == '%' && i + 1 < size &&
                 (str[i + 1] == 'd' || str[i + 1] == 'i' || str[i + 1] ==  's' || str[i + 1] == 'd'))
        {
            str.erase(i, 2);
            str.insert(i, variablesToAdd[varIdx]->node()->value);
            i--;
            size = str.size();
            varIdx++;
        }
        // regular output
        else
            outFS << str[i];
    }
}


/***
 * This parses a user defined function call
 * @param root the current place in the AST
 * @return a string of the return value (if it exists) or an empty string
 */
string interpreter::parseCall(ASTnode * root)
{
    if (root)
    {
        string funcName = root->content();
        populateParamList(root, root->content());
        auto * beginNode = ast->retrieveFunctionPtr(funcName);
        return parseFunction(beginNode);
    }
    return "";
}


/***
 * This safely iterates the iterator to the next node
 * in the AST
 * @param iter the iterator pointer to the AST
 */
void interpreter::nextNode(ASTnode *& iter)
{
    if (iter)
    {
        if (iter->sibling())
            iter = iter->sibling();
        else
            iter = iter->child();
    }
}


/***
 * This determines a variable (term) type for when you are
 * evaluating expressions
 *
 * @param root the current node in the expression
 * @return the type of variable root contains
 */
string interpreter::findVarType(ASTnode * root)
{
    if (root->type().empty()) {
        // signed or unsigned integer
        if (isdigit(root->content()[0]) || root->content()[0] == '-' || root->content()[0] == '+')
            return INT;
            // boolean
        else if (root->content() == "TRUE" || root->content() == "FALSE")
            return BOOL;
            // variable/function call/string/char
        else {
            if (root->node()) {
                if (root->node()->type() == "function")
                    return CALL;
                return VARIABLE;
            } else if (root->sibling())
                return root->sibling()->content() == "'" ? CHAR : STRING;
            return root->type();
        }
    }
    return root->type();

}


/***
 * This evaluates a numerical expression
 *
 * @param operand1 first given operand
 * @param operand2 second given operand
 * @param op the operator
 * @param resultType the datatype of their evaluation
 * @return the result of the expression
 */
string interpreter::evaluateNumerical(ASTnode * operand1, ASTnode * operand2, const string& op, string& resultType)
{
    string op1Type, op2Type;
    string op1val, op2val;
    prepOperands(operand1, operand2, resultType,op1Type, op2Type, op1val, op2val );

    if (op == "+")
    {
        if (op1Type == INT)
        {
            int convertedOp1 = stoi(op1val), convertedOp2 = stoi(op2val);
            return to_string(convertedOp1 + convertedOp2);
        }
        else
        {
            return op1val + op2val;
        }
    }
    else if (op == "-")
    {
        if (op1Type == INT)
        {
            int convertedOp1 = stoi(op1val), convertedOp2 = stoi(op2val);
            return to_string(convertedOp1 - convertedOp2);
        }
        else
        {
            resultType = INT;
            return to_string(op1val[0] - op2val[0]);
        }
    }
    else if (op == "*")
    {
        if (op1Type == INT)
        {
            int convertedOp1 = stoi(op1val), convertedOp2 = stoi(op2val);
            return to_string(convertedOp1 * convertedOp2);
        }
        else
        {
            throw runtime_error("Cannot perform multiplication of strings/chars\n");
        }
    }
    else if (op == "/")
    {
        if (op1Type == INT)
        {
            int convertedOp1 = stoi(op1val), convertedOp2 = stoi(op2val);
            return to_string(convertedOp1 / convertedOp2);
        }
        else
        {
            throw runtime_error("Cannot perform division of strings/chars\n");
        }
    }
    else if (op == "%")
    {
        if (op1Type == INT)
        {
            int convertedOp1 = stoi(op1val), convertedOp2 = stoi(op2val);
            return to_string(convertedOp1 % convertedOp2);
        }
        else
        {
            throw runtime_error("Cannot perform modulo of strings/chars\n");
        }
    }
    else
        throw runtime_error("Unexpected operator");

}


/***
 * This sets up string variables for evaluating expressions
 *
 * @param operand1 first operand
 * @param operand2 second operand
 * @param resultType the type of their result
 * @param op1Type the datatype of the first operand
 * @param op2Type the datatype of the second operand
 * @param op1val the value of the first operand
 * @param op2val the value of the second operand
 */
void interpreter::prepOperands(ASTnode * operand1, ASTnode * operand2, string& resultType,
                               string& op1Type, string& op2Type, string& op1val, string& op2val )
{
    if (operand1)
        op1Type = findVarType(operand1);
    op2Type = findVarType(operand2);

    if (operand1) {
        // check if op1 is a variable
        if (op1Type == VARIABLE) {
            op1Type = operand1->node()->dataType();
            op1val = operand1->node()->value;
            if (op1val == "FALSE")
                op1val = "0";
            else if (op1val == "TRUE")
                op1val = "1";
        }
            // check if op1 is a call
        else if (op1Type == CALL) {
            op1Type = operand1->node()->dataType();
            auto *beginNode = ast->retrieveFunctionPtr(operand1->content());
            op1val = parseCall(beginNode);

        }
        else if (operand1->content() == "FALSE")
            op1val = "0";
        else if (operand1->content() == "TRUE")
            op1val = "1";
        else
            op1val = operand1->content();
    }

    // check is op2 is a variable
    if (op2Type == VARIABLE) {
        op2Type = operand2->node()->dataType();
        op2val = operand2->node()->value;
        if (op2val == "FALSE")
            op2val = "0";
        else if (op2val == "TRUE")
            op2val = "1";
    }
        // check is op2 is a call
    else if (op2Type == CALL) {
        op2Type = operand2->node()->dataType();
        auto *beginNode = ast->retrieveFunctionPtr(operand2->content());
        op2val = parseCall(beginNode);

    }
    else if (operand2->content() == "FALSE")
        op2val = "0";
    else if (operand2->content() == "TRUE")
        op2val = "1";
    else
        op2val = operand2->content();

    // set result type according to op1 (arbitrary)
    resultType = op2Type;
}


/***
 * This safely parses to the next node in an AST
 * @param root current place in the AST
 */
void interpreter::parseToNextLine(ASTnode *& root)
{
    while (root && root->sibling())
        root = root->sibling();
    if (root)
        root = root->child();
}


/***
 * This parses a for loop expression
 * @param root beginning node of a for loop (FOR EXPRESSION 1)
 * @return the result of the statement (if there is a return called), an empty
 * string if not
 */
string interpreter::parseForLoop(ASTnode *&root)
{
    if (root)
    {
        nextNode(root); // iterate past FOR EXPRESSION 1
        evaluateExpression(root);
        nextNode(root); // iterate to FOR EXPRESSION 2
        nextNode(root); // iterate past
        ASTnode * beginning = root, * iteration;
        string result;

        // while the expression is true
        while (evaluateExpression(root) == "1")
        {
            nextNode(root); // parse FOR EXPRESSION 3
            iteration = root; // set pointer to third expression for later
            parseToNextLine(root); // parse to block statement
            result = parseBlockStatement(root);
            evaluateExpression(iteration); // catch the third for expression
            root = beginning;
        }
        goPastBlockStatement(root); // iterate past the loop
        return result;
    }
    return "";
}


/***
 * This parses a while loop
 * @param root first node of the while loop expression (while)
 * @return the result of the statement (if there is a return called), an empty
 * string if not
 */
string interpreter::parseWhileLoop(ASTnode *&root)
{
    if (root)
    {
        nextNode(root); // iterate past while
        ASTnode * beginning = root; // set the beginning to the first node of the condition
        string result;
        while (evaluateExpression(root) == "1" && result.empty())
        {
            result = parseStatement(root);
            root = beginning;
        }
        goPastBlockStatement(root); // iterate past the loop
        return result;
    }
    return "";
}


/***
 * This evaluates a boolean expression
 *
 * @param operand1 first given operand
 * @param operand2 second given operand
 * @param op the operator
 * @param resultType the datatype of their evaluation (always BOOL)
 * @return the result of the expression
 */
string interpreter::evaluateBoolean(ASTnode * operand1, ASTnode * operand2, const string& op, string& resultType)
{
    string op1Type, op2Type;
    string op1val, op2val;
    prepOperands(operand1, operand2, resultType,op1Type, op2Type, op1val, op2val);
    resultType = BOOL;


    if (op == "!")
    {
        int convertedVal = stoi(op2val);
        return convertedVal == 1 ? "0" : "1";
    }
    else if (op == "==")
    {
        return to_string(op1val == op2val);
    }
    else if (op == "<=")
    {
        if (op1Type == INT)
        {
            int convertedOp1 = stoi(op1val), convertedOp2 = stoi(op2val);
            return to_string(convertedOp1 <= convertedOp2);
        }
        else
        {
            return to_string(op1val <= op2val);
        }
    }
    else if (op == ">=")
    {
        if (op1Type == INT)
        {
            int convertedOp1 = stoi(op1val), convertedOp2 = stoi(op2val);
            return to_string(convertedOp1 >= convertedOp2);
        }
        else
        {
            return to_string(op1val >= op2val);
        }
    }
    else if (op == "<")
    {
        if (op1Type == INT)
        {
            int convertedOp1 = stoi(op1val), convertedOp2 = stoi(op2val);
            return to_string(convertedOp1 < convertedOp2);
        }
        else
        {
            return to_string(op1val < op2val);
        }
    }
    else if (op == ">")
    {
        if (op1Type == INT)
        {
            int convertedOp1 = stoi(op1val), convertedOp2 = stoi(op2val);
            return to_string(convertedOp1 > convertedOp2);
        }
        else
        {
            return to_string(op1val > op2val);
        }
    }
    else if (op == "&&")
    {
        if (op2Type == BOOL || op2Type == INT)
        {
            int convertedOp1 = stoi(op1val), convertedOp2 = stoi(op2val);
            return to_string(convertedOp1 && convertedOp2);
        }
        else
        {
            printf("op1 type is %s and op2 type is %s\n", op1Type.c_str(), op2Type.c_str());
            throw runtime_error("Cannot perform AND of strings/chars\n");
        }
    }
    else if (op == "||")
    {
        if (op1Type == BOOL)
        {
            int convertedOp1 = stoi(op1val), convertedOp2 = stoi(op2val);
            return to_string(convertedOp1 || convertedOp2);
        }
        else
        {
            throw runtime_error("Cannot perform OR of strings/chars\n");
        }
    }
    else if (op == "!=")
    {
        if (op1Type == INT)
        {
            int convertedOp1 = stoi(op1val), convertedOp2 = stoi(op2val);
            return to_string(convertedOp1 != convertedOp2);
        }
        else
        {
            return to_string(op1val != op2val);
        }
    }
    else
        throw runtime_error("Unexpected operator");

//    return "";
}


/***
 * This parses just past (one place after) a given character
 * @param root the current node in the AST
 * @param val the character/string to parse past
 */
void interpreter::parsePastChar(ASTnode *& root, const string& val)
{
    while (root && root->content() != val)
        nextNode(root);
    nextNode(root);
}


/***
 * This evaluates a given expression
 * @param root the first node of the expression
 * @return the result of the evaluation
 */
string interpreter::evaluateExpression(ASTnode *& root)
{
    string end_result;
    if (root)
    {
        stack<ASTnode*> varStack;
        vector<ASTnode*> nodesToDelete;
        string type;
        bool addType = false;
        int idx = -1;
        auto * prev = root;
        while (root)
        {
            // quotes are ignored (for strings)
            if (root->content() != "\"" && root->content() != "'")
            {
                if (!isalpha(root->content()[0]) && !isdigit(root->content()[0]) && !isNumber(root->content()))
                {
                    // catch operator
                    string op = root->content();
                    // get first operand
                    ASTnode * operand2 = varStack.top(), *operand1 = nullptr;
                    varStack.pop();
                    // get second operand
                    if (op != "!")
                    {
                        operand1 = varStack.top();
                        varStack.pop();
                    }
                    else {printf("");}
                    // evaluate result and push onto stack
                    string resultType;

                    ASTnode * result;
                    if (op != "=")
                    {
                        // decipher if it is a boolean or numerical expression
                        if (isBooleanOp(root))
                            result = new ASTnode(evaluateBoolean(operand1, operand2, op, resultType), resultType);
                        else
                            result = new ASTnode(evaluateNumerical(operand1, operand2, op, resultType), resultType);

                        // add the node to a list, so it can be deleted later
                        nodesToDelete.push_back(result);
                        varStack.push(result);
                        end_result = result->content();
                    }
                    else
                    {
                        // ensure you are assigning a value to a variable
                        if (operand1 && operand1->node()) {
                            // if you are assigning an index a value
                            if (idx != -1)
                            {
                                operand1->node()->value[idx] = operand2->content()[0];
                            }
                            else
                            {
                                operand1->node()->value = operand2->content();
                            }
                            for (auto * node : nodesToDelete)
                                delete node;
                            return operand1->node()->value;
                        }
                        else
                            throw runtime_error("Attempting to assign a value to a non-variable\n");
                    }

                }
                else {
                    // user defined function call
                    if (root->sibling() && root->sibling()->content() == "(")
                    {
                        auto * returnVal = new ASTnode(parseCall(root), root->node()->dataType());
                        nodesToDelete.push_back(returnVal);
                        varStack.push(returnVal);
                        parsePastChar(root, ")");
                        continue;
                    }
                    else
                    {
                        // this is for updating the type of string/char accordingly
                        if (addType)
                        {
                            root->type(type);
                            type.clear();
                        }
                        varStack.push(root);
                        // this catches the index for future updating
                        if (root->sibling() && root->sibling()->content() == "[")
                        {
                            root = root->sibling(); // parse to [
                            root = root->sibling(); // parse to idx
                            idx = stoi(root->content()); // catch idx
                            root = root->sibling(); // parse to ]
                        }
                    }
                }
            }
            else
            {
                if (!addType) {
                    if (root->content() == "'")
                        type = CHAR;
                    else
                        type = STRING;
                    addType = true;
                }
                else
                    addType = false;
            }
            prev = root;
            root = root->sibling();

        }
        root = prev;
        nextNode(root);
        // deallocate the result nodes
        for (auto * node : nodesToDelete)
            delete node;
        if (!varStack.empty() && end_result.empty())
        {
            string retType = findVarType(varStack.top());
            if (retType == VARIABLE)
            {
                return varStack.top()->node()->value;
            }
            else if (retType == CALL)
            {
                auto * beginNode = ast->retrieveFunctionPtr(varStack.top()->content());
                return parseCall(beginNode);

            }
            else
                return varStack.top()->content();
        }
    }

    return end_result;
}