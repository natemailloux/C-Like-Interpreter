//
// Created by natem on 2/20/2025.
//

#ifndef ASSIGNMENT2_TOKENIZE_H
#define ASSIGNMENT2_TOKENIZE_H
#include "tokenList.h"
#include "token.h"
#include "../symbolTable/symbolTable.h"
#include <string>
#include <vector>
using namespace std;

bool isInt(const string& curText);
int isNumber(const string& curText);
string checkToken(const string& currentText);
bool checkInvalidIntegerError(const string& curText);
int state(int& state, const char& curChar, const char& prevChar, const string& currentText, const int& idx, string cL);
tokenList * createTokenList(const string& inputFile, const string& outputFile);
void outputTokenList(const string& outputFile, token * head);
bool isBoolean(const string& curText);
bool beginningOfBoolean(const string& curText);
string convertChar(const char& curChar);
bool checkReserveWord(const string& curText, const vector<string>& list);
void adjustFiles(string& initInput, string& tokenInput, string& outputFile, const string& idx);


// These are the vectors associated with the special tokens (POSSIBLE_TOKENS being the tokens themselves,
// TOKEN_TITLES being their names, VAR_RESERVED_WORDS being a list of invalid variable names,
// and POSSIBLE_RETURN_TYPES being a list of valid function return types

const vector<string> TOKEN_TITLES = {"L_PAREN", "R_PAREN", "L_BRACE", "R_BRACE", "L_BRACKET", "R_BRACKET", "SEMICOLON",
                                     "ASSIGNMENT_OPERATOR", "MODULO", "PLUS", "MINUS", "DIVIDE", "ASTERISK", "DOUBLE_QUOTE",
                                     "SINGLE_QUOTE", "COMMA", "CARET", "LT", "GT", "LT_EQUAL", "GT_EQUAL", "BOOLEAN_AND",
                                     "BOOLEAN_OR", "BOOLEAN_NOT", "BOOLEAN_EQUAL", "BOOLEAN_NOT_EQUAL", "BOOLEAN_TRUE",
                                     "BOOLEAN_FALSE"};

const vector<string> POSSIBLE_TOKENS = {"(", ")", "{", "}", "[", "]", ";", "=", "%", "+", "-","/", "*", "\"", "\'", ",", "^",
                                        "<", ">", "<=", ">=", "&&", "||", "!", "==", "!=", "true", "false"};


const vector<string> VAR_RESERVED_WORDS = {"alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor",
                                           "bool","break", "case", "catch", "char", "char16_t", "char32_t", "class",
                                           "compl","const", "constexpr", "const_cast", "continue", "decltype",
                                           "default","delete", "do", "double", "dynamic_cast","else", "enum",
                                           "explicit","export", "extern", "false", "float", "for", "friend", "goto",
                                           "if","inline", "int", "long", "mutable", "namespace", "new", "noexcept",
                                           "not", "not_eq", "nullptr", "operator", "or", "or_eq", "register",
                                           "reinterpret_cast", "return", "short",  "signed", "sizeof", "static",
                                           "static_assert", "static_cast", "struct", "switch", "template",
                                           "thread_local", "throw", "true", "typedef", "typeid", "typename", "union",
                                           "unsigned", "virtual", "void", "volatile", "wchar_t", "while",
                                           "xor", "xor_eq"};

const vector<string> FUNCTION_SPECIFIC_RESERVES = {"sizeof", "alignof", "typeid", "decltype", "new", "delete",
                                                   "printf"};


#endif //ASSIGNMENT2_TOKENIZE_H
