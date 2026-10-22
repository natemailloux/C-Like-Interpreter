//
// Created by natem on 4/20/2025.
//

#include <iostream>
#include "tokenize/tokenize.h"
#include "removeComments/removeComments.h"
#include "tokenize/tokenList.h"
#include "interpreter/interpreter.h"
#include "CST/CST.h"
#include "AST/AST.h"
using namespace std;


/***
 * This creates all the structures relevant to simulating the
 * compilation of an input file
 *
 * @param initInput the given input file
 * @param tokenInput file to place the file without comments
 * @param outputFile file to hold the output of the input file
 * @param userNum the test number
 */
void compile(string& initInput, string& tokenInput, string& outputFile, int userNum)
{
    cout << "TEST #" << userNum << endl;
    string idx = to_string(userNum);
    // this adjusts the name of the file to correlate to the set test
    if (idx.size() == 1) {
        initInput[46] = idx[0];
        tokenInput[17] = idx[0];
        outputFile[13] = idx[0];
    } else {
        adjustFiles(initInput, tokenInput, outputFile, idx);
    }
    removeComments(initInput, tokenInput);
    auto *TL = createTokenList(tokenInput, "output/tokenTest");
    auto * ST = new symbolTable();
    auto * cst = new CST(TL->head(), outputFile, ST);
    auto * ast = new AST(cst, ST);
    interpreter runProgram(ast, outputFile);

    if (!cst->hasErrors()) {
        cout << "SUCCESSFUL\n";
        // remove comment to test the output of a set structure
//                ST->output(outputFile);
//            ast->output(outputFile);
//            cst->outputTraversal(outputFile);
    } else {
        cout << "CHECK OUTPUT FILE FOR ERROR(S)\n";
    }
    cout << "OUTPUT CAN BE FOUND AT " << outputFile << endl << endl;

}


/***
 * This runs the test(s) given a test number value
 *
 * @param userNum the test number
 * @param initInput the given input file
 * @param tokenInput file to place the file without comments
 * @param outputFile file to hold the output of the input file
 * @param numTests the amount of tests
 */
void runTests(const int& userNum, string initInput, string tokenInput, string outputFile, int numTests)
{
    if (userNum <= 0)
        throw runtime_error("invalid test value\n");
    else if (userNum <= numTests)
    {
        compile(initInput, tokenInput, outputFile, userNum);

    }
    else {
        for (int i = 1; i <= numTests; i++) {
            compile(initInput, tokenInput, outputFile, i);
        }
    }
}

/***
 * Driver program
 */
int main() {

    #define NUM_TESTS 3
    int userNum = 4;

    // file strings
    string initInput = "init input/programming_assignment_6-test_file_1.c";
    string tokenInput = "token input/input1.txt";
    string outputFile = "output/output1.txt";

//    cout << "Enter a value (1-3) to run a set test, or 4 (or another larger value) to test all the cases\n";
//    cin >> userNum;
    runTests(userNum, initInput, tokenInput, outputFile, NUM_TESTS);

    return 0;
}
