#ifndef AST_H
#define AST_H

#include<bits/stdc++.h>
using namespace std;

enum class ExprType {
    IDENTIFIER,
    NUMBER,
    STRING,
    UNARY,
    BINARY,
    CALL,
    ARRAY_ACCESS,
};

struct Expr {
    ExprType type;
    string value;
    vector<Expr*> children;
};

struct IdentExpr : Expr {
    string name;
    IdentExpr(const string& n) : name(n) {
        type = ExprType::IDENTIFIER;
        value = n;
    }
};

struct NumExpr : Expr {
    string number;
    NumExpr(const string& num) : number(num) {
        type = ExprType::NUMBER;
        value = num;
    }
};

struct StrExpr : Expr {
    string str;
    StrExpr(const string& s) : str(s) {
        type = ExprType::STRING;
        value = s;
    }
};

struct UnaryExpr : Expr {
    string op;
    Expr* operand;
    UnaryExpr(const string& oper, Expr* expr) : op(oper), operand(expr) {
        type = ExprType::UNARY;
        value = oper;
        children.push_back(operand);
    }
};

struct BinExpr : Expr {
    string op;
    Expr* left;
    Expr* right;
    BinExpr(Expr* l, const string& oper, Expr* r) 
        : op(oper), left(l), right(r) {
        type = ExprType::BINARY;
        value = oper;
        children.push_back(left);
        children.push_back(right);
    }
};

struct CallExpr : Expr {
    string funcName;
    vector<Expr*> args;
    CallExpr(const string& fname) : funcName(fname) {
        type = ExprType::CALL;
        value = fname;
    }
};

struct ArrExpr : Expr {
    Expr* arrayName;
    Expr* index;
    ArrExpr(Expr* arr, Expr* idx) 
        : arrayName(arr), index(idx) {
        type = ExprType::ARRAY_ACCESS;
        children.push_back(arrayName);
        children.push_back(index);
    }
};

enum class StmtType {
    EXPRESSION,
    DECLARATION,
    RETURN,
    IF,
    WHILE,
    FOR,
    BLOCK,
    BREAK,
    CONTINUE,
};

struct Stmt {
    StmtType type;
    int line = 0;
    vector<Stmt*> children;
};

struct ExprStmt : Stmt {
    Expr* expr;
    ExprStmt(Expr* e) : expr(e) {
        type = StmtType::EXPRESSION;
    }
};

struct DeclStmt : Stmt {
    string varType;
    string varName;
    Expr* init;
    bool isArray = false;
    int arraySize = 0;
    
    DeclStmt(const string& vType, const string& vName, Expr* i = nullptr)
        : varType(vType), varName(vName), init(i) {
        type = StmtType::DECLARATION;
    }
};

struct RetStmt : Stmt {
    Expr* retExpr;
    RetStmt(Expr* expr = nullptr) : retExpr(expr) {
        type = StmtType::RETURN;
    }
};

struct BlockStmt : Stmt {
    vector<Stmt*> statements;
    BlockStmt() {
        type = StmtType::BLOCK;
    }
};

struct IfStmt : Stmt {
    Expr* cond;
    Stmt* thenPart;
    Stmt* elsePart;
    IfStmt(Expr* c, Stmt* t, Stmt* e = nullptr)
        : cond(c), thenPart(t), elsePart(e) {
        type = StmtType::IF;
    }
};

struct WhileStmt : Stmt {
    Expr* cond;
    Stmt* body;
    WhileStmt(Expr* c, Stmt* b) : cond(c), body(b) {
        type = StmtType::WHILE;
    }
};

struct ForStmt : Stmt {
    Stmt* init;
    Expr* cond;
    Expr* inc;
    Stmt* body;
    ForStmt(Stmt* i, Expr* c, Expr* inc, Stmt* b)
        : init(i), cond(c), inc(inc), body(b) {
        type = StmtType::FOR;
    }
};

struct BreakStmt : Stmt {
    BreakStmt() {
        type = StmtType::BREAK;
    }
};

struct ContinueStmt : Stmt {
    ContinueStmt() {
        type = StmtType::CONTINUE;
    }
};

struct Function {
    string retType;
    string name;
    vector<pair<string, string>> params;
    BlockStmt* body;
    int line = 0;
    
    Function() : body(nullptr) {}
};

#endif