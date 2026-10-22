//
// Created by natem on 3/10/2025.
//

#ifndef ASSIGNMENT4_STNODE_H
#define ASSIGNMENT4_STNODE_H
#include <string>
#include <utility>
using namespace std;
class symbolTable;


class stNode {

public:

    // non-arrays
    stNode(string name, string _type, string dataType, int programScope): _idName{std::move(name)},
     _idType{std::move(_type)}, _dataType{std::move(dataType)}, _isArr{false}, _arrSize{0}, _scope{programScope},
     _next{nullptr}, _paramList{nullptr} {}

    // arrays
    stNode(string name, string _type, string dataType, bool isArray, int size, int programScope):
    _idName{std::move(name)},_idType{std::move(_type)}, _dataType{std::move(dataType)}, _isArr{isArray}, _arrSize{size},
    _scope{programScope}, _next{nullptr}, _paramList{nullptr} {}

    // functions
    void next(stNode * nextNode) { _next = nextNode; }
    void addST(symbolTable * _st) { _paramList = _st; }

    stNode * next() { return _next; }
    symbolTable * paramList() { return _paramList; }

    string idName() { return _idName; }
    string type() { return _idType; }
    string dataType() { return _dataType; }


    [[nodiscard]] bool isArr() const { return _isArr; }
    [[nodiscard]] int size() const { return _arrSize; }
    [[nodiscard]] int scope() const { return _scope; }
    string value;

private:

    string _idName;
    string _idType;
    string _dataType;
    bool _isArr;
    int _arrSize;
    int _scope;
    stNode * _next;
    symbolTable * _paramList;

};


#endif //ASSIGNMENT4_STNODE_H
