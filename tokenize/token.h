//
// Created by natem on 2/4/2025.
//

#ifndef ASSIGNMENT2_TOKEN_H
#define ASSIGNMENT2_TOKEN_H
#include <string>
#include <utility>
using namespace std;


class token {

public:
    token() { _next = nullptr; }
    token(string _type, string _content): type(std::move(_type)), _content(_content) {}
    void next(token * _token) { _next = _token; }
    token * next() { return _next; }
    string content() { return _content; }
    string _type() { return type; }
    bool isEndlToken() { return type == "endl"; }
    bool isScope() { return type == "scope";}
    bool startScope() { return type == "startScope"; }
    bool endScope() { return type == "endScope"; }

private:
    string type;
    string _content;
    token * _next{};
};


#endif //ASSIGNMENT2_TOKEN_H
