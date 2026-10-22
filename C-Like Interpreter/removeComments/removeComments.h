//
// Created by natem on 2/20/2025.
//

#ifndef ASSIGNMENT2_REMOVECOMMENTS_H
#define ASSIGNMENT2_REMOVECOMMENTS_H
#include <string>
using namespace std;

void checkState(const char& currentChar, const char& prevChar, int& state, const int& idx, const string& CL);
void removeComments(const string& inputFile, const string& outputFile);


#endif //ASSIGNMENT2_REMOVECOMMENTS_H
