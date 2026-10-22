#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
using namespace std;

/***
 * 0: regular state/initial state
 * 1: one slash (in comment)
 * 2: c++ style comment
 * 3: c style
 * 4: in single quoted string
 * 5: in double quoted string
 * 6: buffer state (for end of c style comment)
 * 7: c style error state
 */

void checkState(const char& currentChar, const char& prevChar, int& state, const int& idx, const string& CL)
{

    switch(state)
    {
        // initial state
        case 0:
            if (currentChar == '/')
            {
                // current character is not the end of the current line
                if (idx + 1 < CL.size())
                {
                    char nextChar = CL.at(idx + 1);
                    // check if this is the beginning of comment(s)
                    if (nextChar == '/' || nextChar == '*')
                    {
                        state = 1;
                    }
                }
            }
                // check if going to the single quote state
            else if (currentChar == '\'')
            {
                state = 4;
            }
                // check if going to the double quote state
            else if (currentChar == '\"')
            {
                state = 5;
            }
            // check for c style error
            if (prevChar == '*' && currentChar == '/')
            {
                state = 7;
            }
            break;
        case 1: // one slash state ( checking what type of comment )
            // c++ style
            if (currentChar == '/') {
                state = 2;
            }
                // c style
            else if (currentChar == '*')
            {
                state = 3;
            }
            break;
        case 3: // c style comment state
            // ensure the block has not ended
            if (prevChar == '*' && currentChar == '/')
            {
                state = 6;
            }
            break;
        case 4: // single quote state
            if (currentChar == '\'')
            {
                state = 0;
            }
            break;
        case 5: // double quote state
            if (currentChar == '\"')
            {
                state = 0;
            }
            break;
        case 6: // buffer state for c style comments
        {
            state = 0;
        }
        default:
        {}
    }
}


void removeComments(const string& inputFile, const string& outputFile)
{
    ifstream inFS;
    ofstream outFS;
    string currentLine;
    stringstream lineStream;
    int currentLineNum = 1, startCommNum = 0, stateNum = 0;
    char currentChar, prevChar = '\0';

    inFS.open(inputFile);
    outFS.open(outputFile);

    // ensure both files opened correctly
    if (!inFS.is_open() || !outFS.is_open())
    {
        string file;
        if (!outFS)
            file = outputFile;
        else
            file = inputFile;
        cout << "ERROR OPENING FILE " << file << endl;
        exit(1);
    }

    while (getline(inFS, currentLine))
    {

        for (int idx = 0; idx < currentLine.size(); idx++)
        {
            currentChar = currentLine.at(idx);
            checkState(currentChar, prevChar, stateNum, idx, currentLine);
            switch(stateNum)
            {
                case 1: // comment starts
                {
                    startCommNum = currentLineNum;
                }
                case 2: case 3: case 6: // in comment
                {
                    outFS << " ";
                    break;
                }
                case 7: // c style error detected !
                {
                    startCommNum = currentLineNum;
                    goto checkError;
                }
                default:
                    outFS << currentChar;
            }

            prevChar = currentChar;

        }
        // reset the state if it's not the c style or error state
        if (stateNum != 3 && stateNum != 7)
        {
            stateNum = 0;
        }

        prevChar = '\0';
        currentLineNum++;
        outFS << "\n";

    }

    checkError:
    // c style comment not terminated or error in user implementation
    if (stateNum == 3 || stateNum == 7)
        cout << outputFile << " " << "ERROR: Program contains C-style, unterminated comment on line " << startCommNum << endl;


    inFS.close();
    outFS.close();
}