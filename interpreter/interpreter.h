//
// Created by natem on 4/22/2025.
//

#ifndef ASSIGNMENT5_INTERPRETER_H
#define ASSIGNMENT5_INTERPRETER_H
#include "../AST/AST.h"


#define INT "int"
#define VARIABLE "variable"
#define CALL "call"
#define CHAR "char"
#define BOOL "bool"


class interpreter {

public:
    interpreter(AST * _ast, const string& outputFile);

    string evaluateExpression(ASTnode *& root);
    string findVarType(ASTnode * root);
    string evaluateNumerical(ASTnode * operand1, ASTnode * operand2, const string& op, string& resultType);
    string evaluateBoolean(ASTnode * operand1, ASTnode * operand2, const string& op, string& resultType);
    string parseCall(ASTnode * root);
    string parseFunction(ASTnode *& root);
    string parseStatement(ASTnode *& root);
    string parseCompoundStatement(ASTnode *& root);
    string parseBlockStatement(ASTnode *& root);
    string parseReturnStatement(ASTnode *& root);
    string parseConditional(ASTnode *& root);
    string parseWhileLoop(ASTnode *& root);
    string parseForLoop(ASTnode *& root);

    bool isBooleanOp(ASTnode * root);
    bool escapeCharacter(const char& next, const string& CL, int idx, string& escape);

    void nextNode(ASTnode *& iter);
    void goPastBlockStatement(ASTnode *& root);
    void parseToNextLine(ASTnode *& root);
    void populateParamList(ASTnode *& iter, const string& function);
    void prepOperands(ASTnode * operand1, ASTnode * operand2, string& resultType,
                      string& op1Type, string& op2Type, string& op1val, string& op2val );
    void outputPrintf(string& str, const vector<ASTnode*>& variablesToAdd);
    void parsePrintf(ASTnode *& root);
    void parseAssignment(ASTnode *& root);
    void parsePastChar(ASTnode *& root, const string& val);


private:
    AST * ast;
    ofstream outFS;
};


#endif //ASSIGNMENT5_INTERPRETER_H
