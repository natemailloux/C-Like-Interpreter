//
// Created by natem on 2/17/2025.
//

#include <iostream>
#include<string>
#include "tokenize.h"
#include <vector>
#include <fstream>
#include <cassert>
#include <algorithm>
using namespace std;


/***
 * This iterates through a given vector and checks if the given text is one of the elements of that
 * vector (using any_of)
 * @param curText string to be checked
 * @param list vector with elements to be compared to
 * @return true if curText is equal to one of the elements, false if not
 */
bool checkReserveWord(const string& curText, const vector<string>& list)
{
    return any_of(list.begin(), list.end(), [&curText](const string& i)
    {return i == curText;});

}


/***
 * This converts a character into a string that solely contains that character
 * @param curChar given char value
 * @return a string with only that character in it
 */
string convertChar(const char& curChar)
{
    string converted;
    return converted + curChar;
}


/***
 * This function checks if a non-empty string is solely a signed/unsigned integer (in a string)
 * by traversing each index of the array and calling isNumber (a helper function)
 * @param curText the string to be traversed
 * @return the related title to the token if it is a special token, an empty string if not
 */
bool isInt(const string& curText)
{
    assert(!curText.empty());
    int count = 0, startIdx = 0;
    unsigned long long size = curText.size();

    if (curText[0] == '-' || curText[0] == '+')
    {
        startIdx = 1;
    }


    for (int i = startIdx; i < size; i++)
    {
        char curChar = curText[i];
        if (isNumber(convertChar(curChar)))
        {
            count++;

        }

    }
    // this ensures that the count is at least 1 (string is not solely a minus/plus sign)
    // and the amount of non-sign characters is the count (there is only one count)
    return (count == size - startIdx && count > 0);
}


/***
 * This function checks if a given character (converted to a string) is a portion of a boolean expression
 * @param curText converted character to be checked
 * @return true if it is one of the characters, false if not
 */
bool isBoolean(const string& curText)
{
    return curText == ">"|| curText == "=" || curText == "<" || curText == "|" || curText == "&";
}


/***
 * This function checks a string to see if it is possible to convert it to an integer, meaning that it will return
 * true if the string only contains integers (and sign if applicable)
 * @param curText the string to viewed
 * @return true if the string only contains integer/related values, false if not
 */
int isNumber(const string& curText)
{
    try
    {
        stoi(curText);
    }
    catch (const invalid_argument& e)
    {
        return false;
    }

    return true;
}


/***
 * This function checks an array of special characters to see if the current char is one of them
 * @param currentText
 * @return
 */
string checkToken(const string& currentText)
{
    // ensures that the vectors are the same size
    assert(TOKEN_TITLES.size() == POSSIBLE_TOKENS.size());
    if (!currentText.empty() && currentText != " ")
    {
        for (int idx = 0; idx < POSSIBLE_TOKENS.size(); idx++)
        {
            if (currentText == POSSIBLE_TOKENS[idx])
            {
                return TOKEN_TITLES[idx];
            }

        }
    }
    return "";

}


/***
 * This function checks if the a non-int string (a string that is not strictly
 * signed integer) contains an integer value at its zero index (first element)
 * @pre isInt returns false for the entire string as an input
 * @param curText the string to be checked
 * @return true if there index 0 of the string is an integer, false if not
 */
bool checkInvalidIntegerError(const string& curText)
{
    return isNumber(convertChar(curText[0]));
}


/**
 * This function checks if the current char is "&" or "|", this is because these
 * characters alone are not special characters
 * @param curText string to be viewed
 * @return true if it is equal to one of them
 */
bool beginningOfBoolean(const string& curText)
{
    return curText == "&" || curText == "|";
}


/***
 * This function determines the next state given parameters related to the
 * current place in the program
 *
 * @param state the given state
 * @param curChar the current char
 * @param prevChar previous char
 * @param currentText a holder for multi-character tokens
 * @param idx current idx in the current line of the program
 * @param cL current line string
 * @return the next state
 *
 *  * STATE KEY
 * 0: Normal state
 * 1: single char
 * 2: identifier
 * 3: double quote
 * 4: single quote
 * 5: buffer (end of line or regular token)
 * 6: quote buffer
 * 7: string
 * 8: quote
 * 9: Boolean Expression
 * 10: Boolean Buffer
 * 11: end of line
 * 12: true/false buffer
 * 13: bracket (for array size/idx)
 * 14: out of bracket (check for error)
 */
int state(int& state, const char& curChar, const char& prevChar, const string& currentText,
          const int& idx, string cL)
{

    if (state != 0) {

        switch (state) {

            // currently in double quote
            case 3: {
                if (prevChar != '\\') {
                    if (curChar == '\"')
                        state = 6;
                }
                break;
            }
                // currently in single quote
            case 4: {
                if (prevChar != '\\') {
                    if (curChar == '\'')
                        state = 6;
                }
                break;
            }
                // buffer state
            case 5:
            case 6: {
                state = 7;
                break;
            }
                // starting a string
            case 8: {
                if (prevChar == '\"') {
                    if (curChar == '\"') {
                        state = 1;
                    } else {
                        state = 3;
                    }
                    break;
                } else if (prevChar == '\'') {
                    if (curChar == '\'') {
                        state = 1;
                    } else {
                        state = 4;
                    }
                    break;
                } else if (prevChar == '[') {
                    if (curChar == ']') {
                        state = 1;
                        break;
                    } else {
                        state = 13;
                    }

                }

            }
            // in bracket
            case 13: {

                if (curChar == ']') {
                    state = 14;
                }
                break;
            }
            // boolean
            case 9: {
                state = 10;
                break;
            }
            // reset to 0
            default: {
                state = 0;
            }

        }
    }

    if (state == 0)
    {

        if (!isalpha(curChar) && !isdigit(curChar))
        {
            string convertedChar = convertChar(curChar), convertedNextIdx;
            // this is only referenced if it is known that idx + 1 < size, it is placed above to avoid duplicate code
            convertedNextIdx += cL[idx + 1];

            if (beginningOfBoolean(convertedChar) || !checkToken(convertedChar).empty())
            {


                if (curChar == '[' || ((curChar == '\"' || curChar == '\'') && prevChar != '\\'))
                {
                    state = 8;
                }
                else if (curChar == '-')
                {
                    if (idx + 1 < cL.size())
                    {
                        if (isInt(convertedNextIdx))
                            state = 0;
                        else
                            state = 1;
                    }
                    else
                    {
                        state = 1;
                    }

                }
                else if (isBoolean(convertedChar)) {

                    if (idx + 1 < cL.size() && isBoolean(convertedNextIdx))
                    {
                        state = 9;
                    }
                    else
                    {
                        state = 1;
                    }
                }
                else if (curChar == ';' || !currentText.empty())
                {
                    state = 5;
                }
                else
                {
                    state = 1;
                }

            }
            else if (curChar == ' ')
            {
                state = 2;
            }

        }
        else if (isalpha(curChar) && idx == cL.size() - 1)
        {
            state = 11;
        }


    }
    return state;


}


/***
 * This function iterates through a program and creates a token list of
 * its contents
 * @param inputFile the file that holds the program
 * @param outputFile the file that will hold the eventual output if no errors
 * are encountered
 * @return a pointer to the head node of the token list, or null if the file is
 * empty/an error is encountered
 */
tokenList * createTokenList(const string& inputFile, const string& outputFile)
{
    ifstream inFS;
    ofstream outFS;
    string curText, curLine, title, convertedChar;
    int stateNum = 0, lineNum = 1, bracketCtr = 0, parenthesesCtr = 0;
    char curChar, prevChar;
    bool variableDeclaration, functionDeclaration, funcName;
    // error flags
    bool integerError = false;

    auto * TL = new tokenList;
    token *_token = nullptr;
    bool startFunction = false, inFunction = false;
    vector<string> variableNames;

    inFS.open(inputFile);
    outFS.open(outputFile);


    // ensure the files opened correctly
    if (!inFS.is_open() || !outFS.is_open())
    {
        string file;
        if (!inFS.is_open())
            file = inputFile;
        else
            file = outputFile;
        cout << "ERROR OPENING -> " << file << endl;
        exit(1);
    }


    while (getline(inFS, curLine)) {

        // reset curText, state, and booleans
        curText.clear(); // empty the string
        stateNum = 0;

        variableDeclaration = false;
        functionDeclaration = false;
        funcName = false;

        for (int i = 0; i < curLine.size(); i++)
        {
            // get the current char and related state
            curChar = curLine[i];
            stateNum = state(stateNum, curChar, prevChar, curText, i, curLine);

            switch (stateNum) {

                // this is for special tokens (found in the const vector placed at the top)
                case 1: case 7: case 8: case 10:
                {
                    if (stateNum == 8)
                    {
                        if (!curText.empty()) {
                            _token = new token("IDENTIFIER", curText);
                            TL->insert(_token);
                        }
                        curText.clear();
                    }
                    // if the current token is not a non-single char boolean (==, >=, etc)
                    if (stateNum < 10)
                    {
                        convertedChar.clear();
                    }
                    else
                    {
                        convertedChar = curText;
                    }

                    convertedChar += curChar;
                    _token = new token(checkToken(convertedChar), convertedChar);

                    if (stateNum != 8)
                    {
                        stateNum = 0;
                    }
                    curText.clear(); // empty the string
                    break;

                }
                // end of the line (for conditionals)
                case 11:
                {
                    curText += curChar;
                }
                // non-special character token
                case 2: case 5:
                {
                    // ensure that you have characters in curText
                    if (!curText.empty())
                    {
                        string tokenTitle;
                        // check if the curText is an integer (in a string)
                        if (curText == "function" || curText == "procedure")
                        {
                            startFunction = true;
                            if (curText == "function")
                                functionDeclaration = true;
                            TL->insert(_token);
                        }
                        else if (functionDeclaration)
                        {
                            if (isInt(curText))
                            {
                                integerError = true;
                                goto intError;
                            }
                            functionDeclaration = false;
                            variableDeclaration = true;
                            funcName = true;

                        }
                        else if (!variableDeclaration && checkReserveWord(curText, VAR_RESERVED_WORDS))
                        {
                            variableDeclaration = true;
                        }
                        else if (variableDeclaration)
                        {

                            // check for invalid integer placement in an identifier
                            if (isInt(curText))
                            {
                                integerError = true;
                                goto intError;
                            }
                            if (!funcName)
                                variableNames.push_back(curText);

                            variableDeclaration = funcName =  false;

                        }
                        if (isInt(curText))
                        {
                            tokenTitle = "INTEGER";
                        }
                        // string is not entirely an integer, check if invalid integer placement in name
                        else if (checkInvalidIntegerError(curText))
                        {
                            integerError = true;
                            goto intError;
                        }
                        else
                        {
                            if (tokenTitle.empty())
                                tokenTitle = "IDENTIFIER";
                        }

                        _token = new token(tokenTitle, curText);
                        curText.clear(); // empty the string
                    }
                    // this is for the last token prior a semicolon or text in a parentheses for example
                    if (stateNum == 5)
                        i--;
                    break;

                }
                // in a string (either double or single quote)
                case 3: case 4:
                {
                    curText += curChar;
                    break;
                }
                // end of string (decrement to get the last double/single quote)
                case 6:
                {
                    _token = new token("STRING", curText);
                    curText.clear(); // empty the string
                    i--;
                    break;
                }
                case 14:
                {
                    // array index is either a number or an identifier
                    if (isInt(curText))
                    {
                        _token = new token("INTEGER", curText);
                    }
                    else
                        _token = new token("IDENTIFIER", curText);
                    curText.clear(); // empty the string

                }
                // add the char to curText if it is not a space
                default:
                {
                    if (curChar != ' ')
                        curText += curChar;
                }

            }

            // if you have a token, add it to the list
            if (_token)
            {
                TL->insert(_token);

            }
            prevChar = curChar;

        }


        if (startFunction && bracketCtr != 0)
        {
            inFunction = true;
            startFunction = false;
        }
        else if (inFunction && bracketCtr == 0)
        {
            inFunction = false;
        }


        // current line is over
        lineNum++;
        _token = new token("endl", "");
        TL->insert( _token);


    }
    inFS.close();

    intError:
    if (integerError)
    {
        outFS  << "Syntax error on line " << lineNum << ": invalid integer\n";
        return nullptr;
    }

    outFS.close();
    TL->output(outputFile);

    return TL;

}


/***
 * This adjusts the file strings for values that are two digits
 * @param initInput
 * @param tokenInput
 * @param outputFile
 * @param idx
 */
void adjustFiles(string& initInput, string& tokenInput, string& outputFile, const string& idx)
{
    initInput[46] = '\0';
    initInput.insert(46, idx);
    initInput.erase(initInput.begin() + 48);

    tokenInput[17] = '\0';
    tokenInput.insert(17, idx);
    tokenInput.erase(tokenInput.begin() + 19);

    outputFile[13] = '\0';
    outputFile.insert(13, idx);
    outputFile.erase(outputFile.begin() + 15);
}


