#ifndef GRAMMAR_H
#define GRAMMAR_H

#include<bits/stdc++.h>
using namespace std;

namespace Grammar {

class Info {
public:
    static bool isType(const string& w) {
        static const unordered_set<string> types = {
            "int", "long", "short", "char", "float", "double", "void"
        };
        return types.count(w) > 0;
    }
    
    static int getPrecedence(const string& op) {
        static const unordered_map<string, int> prec = {
            {"=", 1}, {"+=", 1}, {"-=", 1}, {"*=", 1}, {"/=", 1}, {"%=", 1},
            {"&=", 1}, {"|=", 1}, {"^=", 1}, {"<<=", 1}, {">>=", 1},
            {"||", 2}, {"&&", 3}, {"|", 4}, {"^", 5}, {"&", 6},
            {"==", 7}, {"!=", 7},
            {"<", 8}, {">", 8}, {"<=", 8}, {">=", 8},
            {"<<", 9}, {">>", 9},
            {"+", 10}, {"-", 10},
            {"*", 11}, {"/", 11}, {"%", 11}
        };
        auto it = prec.find(op);
        return it != prec.end() ? it->second : -1;
    }
    
    static bool isRightAssoc(const string& op) {
        return op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=" || 
               op == "%=" || op == "&=" || op == "|=" || op == "^=" || op == "<<=" || op == ">>=";
    }
};

}

#endif