#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "Ast.h"
#include <bits/stdc++.h>
using namespace std;

class Parser {
public:
    Parser(const vector<Token>& toks) : tokens(toks), position(0) {}

    vector<Function*> parse();
    Expr* parseExpr();

private:
    const vector<Token>& tokens;
    int position;

    Token peek(int offset = 0) const {
        int idx = position + offset;
        return idx < (int)tokens.size() ? tokens[idx] : tokens.back();
    }

    Token get() {
        if (position >= (int)tokens.size()) return tokens.back();
        return tokens[position++];
    }

    bool match(TokenType type, const string& text = "") {
        Token tok = peek();
        if (tok.type == type && (text.empty() || tok.text == text)) {
            get();
            return true;
        }
        return false;
    }

    bool check(TokenType type, const string& text = "") const {
        Token tok = peek();
        return tok.type == type && (text.empty() || tok.text == text);
    }

    Expr* parsePrimary();
    Expr* parseUnary();
    Expr* parseBinaryRHS(int prec, Expr* lhs);
    Expr* parsePostfix(Expr* expr);
    int getPrec();

    Stmt* parseStmt();
    Stmt* parseBlock();
    Stmt* parseIf();
    Stmt* parseWhile();
    Stmt* parseFor();
    Stmt* parseReturn();
    Stmt* parseBreak();
    Stmt* parseContinue();
    Stmt* parseDecl();
    Stmt* parseExprStmt();

    Function* parseFunc();

    bool isType(const string& w) const {
        return w == "int" || w == "long" || w == "short" ||
               w == "char" || w == "float" || w == "double" || w == "void";
    }
};

Expr* Parser::parsePrimary() {
    Token tok = peek();

    if (tok.type == TokenType::IDENTIFIER) {
        string name = get().text;
        if (check(TokenType::PUNCTUATION, "(")) {
            get();
            CallExpr* call = new CallExpr(name);
            if (!check(TokenType::PUNCTUATION, ")")) {
                while (true) {
                    Expr* arg = parseExpr();
                    if (arg) call->args.push_back(arg);
                    if (!match(TokenType::PUNCTUATION, ",")) break;
                }
            }
            match(TokenType::PUNCTUATION, ")");
            return call;
        }
        return new IdentExpr(name);
    }

    if (tok.type == TokenType::NUMBER) return new NumExpr(get().text);
    if (tok.type == TokenType::STRING) return new StrExpr(get().text);
    if (tok.type == TokenType::CHAR) return new StrExpr(get().text);  // Handle CHAR tokens

    if (match(TokenType::PUNCTUATION, "(")) {
        Expr* expr = parseExpr();
        match(TokenType::PUNCTUATION, ")");
        return expr;
    }

    return nullptr;
}

Expr* Parser::parsePostfix(Expr* expr) {
    while (check(TokenType::PUNCTUATION, "[")) {
        get();
        Expr* idx = parseExpr();
        match(TokenType::PUNCTUATION, "]");
        expr = new ArrExpr(expr, idx);
    }
    if (check(TokenType::OPERATOR, "++") || check(TokenType::OPERATOR, "--")) {
        string op = get().text;
        expr = new UnaryExpr(op, expr);
    }
    return expr;
}

Expr* Parser::parseUnary() {
    Token tok = peek();
    
    // Handle sizeof operator
    if (tok.type == TokenType::KEYWORD && tok.text == "sizeof") {
        get();  // consume "sizeof"
        if (check(TokenType::PUNCTUATION, "(")) {
            get();  // consume "("
            // Skip the contents inside sizeof() until we find the matching ")"
            int parenCount = 1;
            while (parenCount > 0 && !check(TokenType::END_OF_FILE)) {
                Token t = peek();
                if (t.type == TokenType::PUNCTUATION && t.text == "(") {
                    parenCount++;
                } else if (t.type == TokenType::PUNCTUATION && t.text == ")") {
                    parenCount--;
                    if (parenCount == 0) {
                        get();  // consume the final ")"
                        break;
                    }
                }
                get();
            }
        }
        // Return a dummy number expression for sizeof
        return new NumExpr("1");
    }
    
    if (tok.type == TokenType::OPERATOR &&
        (tok.text == "+" || tok.text == "-" || tok.text == "!" ||
         tok.text == "~" || tok.text == "++" || tok.text == "--" ||
         tok.text == "*" || tok.text == "&")) {
        get();
        Expr* operand = parseUnary();
        if (!operand) return nullptr;
        return new UnaryExpr(tok.text, operand);
    }
    Expr* expr = parsePrimary();
    if (expr) expr = parsePostfix(expr);
    return expr;
}

int Parser::getPrec() {
    Token tok = peek();
    if (tok.type != TokenType::OPERATOR) return -1;
    static const unordered_map<string, int> prec = {
        {"=",1},{"+=",1},{"-=",1},{"*=",1},{"/=",1},{"%=",1},
        {"&=",1},{"|=",1},{"^=",1},{"<<=",1},{">>=",1},
        {"||",2},{"&&",3},{"|",4},{"^",5},{"&",6},
        {"==",7},{"!=",7},
        {"<",8},{">",8},{"<=",8},{">=",8},
        {"<<",9},{">>",9},
        {"+",10},{"-",10},
        {"*",11},{"/",11},{"%",11}
    };
    auto it = prec.find(tok.text);
    return it != prec.end() ? it->second : -1;
}

Expr* Parser::parseBinaryRHS(int exprPrec, Expr* lhs) {
    while (true) {
        int tokPrec = getPrec();
        if (tokPrec < exprPrec) return lhs;
        Token opTok = get();
        Expr* rhs = parseUnary();
        if (!rhs) { delete lhs; return nullptr; }
        int nextPrec = getPrec();
        if (tokPrec < nextPrec) {
            rhs = parseBinaryRHS(tokPrec + 1, rhs);
            if (!rhs) { delete lhs; return nullptr; }
        }
        lhs = new BinExpr(lhs, opTok.text, rhs);
    }
}

Expr* Parser::parseExpr() {
    Expr* lhs = parseUnary();
    if (!lhs) return nullptr;
    return parseBinaryRHS(0, lhs);
}

Stmt* Parser::parseBlock() {
    if (!match(TokenType::PUNCTUATION, "{")) return nullptr;
    BlockStmt* block = new BlockStmt();
    block->line = peek().lineNumber;
    while (!check(TokenType::PUNCTUATION, "}") && !check(TokenType::END_OF_FILE)) {
        Stmt* s = parseStmt();
        if (s) block->statements.push_back(s);
    }
    match(TokenType::PUNCTUATION, "}");
    return block;
}

Stmt* Parser::parseIf() {
    int ln = peek().lineNumber;
    match(TokenType::KEYWORD, "if");
    match(TokenType::PUNCTUATION, "(");
    Expr* cond = parseExpr();
    match(TokenType::PUNCTUATION, ")");
    Stmt* thenPart = parseStmt();
    Stmt* elsePart = nullptr;
    if (match(TokenType::KEYWORD, "else")) elsePart = parseStmt();
    IfStmt* s = new IfStmt(cond, thenPart, elsePart);
    s->line = ln;
    return s;
}

Stmt* Parser::parseWhile() {
    int ln = peek().lineNumber;
    match(TokenType::KEYWORD, "while");
    match(TokenType::PUNCTUATION, "(");
    Expr* cond = parseExpr();
    match(TokenType::PUNCTUATION, ")");
    Stmt* body = parseStmt();
    WhileStmt* s = new WhileStmt(cond, body);
    s->line = ln;
    return s;
}

Stmt* Parser::parseFor() {
    int ln = peek().lineNumber;
    match(TokenType::KEYWORD, "for");
    match(TokenType::PUNCTUATION, "(");

    Stmt* init = nullptr;
    if (!check(TokenType::PUNCTUATION, ";")) {
        if (isType(peek().text)) {
            init = parseDecl();
        } else {
            Expr* e = parseExpr();
            match(TokenType::PUNCTUATION, ";");
            if (e) init = new ExprStmt(e);
        }
    } else {
        match(TokenType::PUNCTUATION, ";");
    }

    Expr* cond = nullptr;
    if (!check(TokenType::PUNCTUATION, ";")) cond = parseExpr();
    match(TokenType::PUNCTUATION, ";");

    Expr* inc = nullptr;
    if (!check(TokenType::PUNCTUATION, ")")) inc = parseExpr();
    match(TokenType::PUNCTUATION, ")");

    Stmt* body = parseStmt();
    ForStmt* s = new ForStmt(init, cond, inc, body);
    s->line = ln;
    return s;
}

Stmt* Parser::parseReturn() {
    int ln = peek().lineNumber;
    match(TokenType::KEYWORD, "return");
    Expr* expr = nullptr;
    if (!check(TokenType::PUNCTUATION, ";")) expr = parseExpr();
    match(TokenType::PUNCTUATION, ";");
    RetStmt* s = new RetStmt(expr);
    s->line = ln;
    return s;
}

Stmt* Parser::parseBreak() {
    int ln = peek().lineNumber;
    match(TokenType::KEYWORD, "break");
    match(TokenType::PUNCTUATION, ";");
    BreakStmt* s = new BreakStmt();
    s->line = ln;
    return s;
}

Stmt* Parser::parseContinue() {
    int ln = peek().lineNumber;
    match(TokenType::KEYWORD, "continue");
    match(TokenType::PUNCTUATION, ";");
    ContinueStmt* s = new ContinueStmt();
    s->line = ln;
    return s;
}

Stmt* Parser::parseDecl() {
    int ln = peek().lineNumber;
    string varType = get().text;
    
    // Parse first variable
    string varName = get().text;
    DeclStmt* decl = new DeclStmt(varType, varName);
    decl->line = ln;

    // Handle array declarations with multiple dimensions
    while (check(TokenType::PUNCTUATION, "[")) {
        get();
        decl->isArray = true;
        // Parse array size expression
        if (!check(TokenType::PUNCTUATION, "]")) {
            Expr* sizeExpr = parseExpr();
            if (sizeExpr && sizeExpr->type == ExprType::NUMBER && decl->arraySize == 0) {
                decl->arraySize = stoi(sizeExpr->value);
            }
        }
        match(TokenType::PUNCTUATION, "]");
    }

    // Handle initializers
    if (match(TokenType::OPERATOR, "=")) {
        if (check(TokenType::PUNCTUATION, "{")) {
            // Skip initializer list
            get();  // consume "{"
            int braceCount = 1;
            while (braceCount > 0 && !check(TokenType::END_OF_FILE)) {
                if (check(TokenType::PUNCTUATION, "{")) braceCount++;
                else if (check(TokenType::PUNCTUATION, "}")) braceCount--;
                get();
            }
        } else {
            decl->init = parseExpr();
        }
    }
    
    // Handle multiple variable declarations separated by commas
    while (match(TokenType::PUNCTUATION, ",")) {
        varName = get().text;
        DeclStmt* nextDecl = new DeclStmt(varType, varName);
        nextDecl->line = ln;
        
        // Handle arrays in comma-separated declarations
        while (check(TokenType::PUNCTUATION, "[")) {
            get();
            nextDecl->isArray = true;
            if (!check(TokenType::PUNCTUATION, "]")) {
                Expr* sizeExpr = parseExpr();
                if (sizeExpr && sizeExpr->type == ExprType::NUMBER && nextDecl->arraySize == 0) {
                    nextDecl->arraySize = stoi(sizeExpr->value);
                }
            }
            match(TokenType::PUNCTUATION, "]");
        }
        
        if (match(TokenType::OPERATOR, "=")) nextDecl->init = parseExpr();
    }
    
    match(TokenType::PUNCTUATION, ";");
    return decl;
}

Stmt* Parser::parseExprStmt() {
    int ln = peek().lineNumber;
    Expr* expr = parseExpr();
    match(TokenType::PUNCTUATION, ";");
    if (!expr) return nullptr;
    ExprStmt* s = new ExprStmt(expr);
    s->line = ln;
    return s;
}

Stmt* Parser::parseStmt() {
    if (check(TokenType::PUNCTUATION, "{"))    return parseBlock();
    if (check(TokenType::KEYWORD, "if"))       return parseIf();
    if (check(TokenType::KEYWORD, "while"))    return parseWhile();
    if (check(TokenType::KEYWORD, "for"))      return parseFor();
    if (check(TokenType::KEYWORD, "return"))   return parseReturn();
    if (check(TokenType::KEYWORD, "break"))    return parseBreak();
    if (check(TokenType::KEYWORD, "continue")) return parseContinue();
    if (isType(peek().text) && peek(1).type == TokenType::IDENTIFIER) return parseDecl();
    return parseExprStmt();
}

Function* Parser::parseFunc() {
    Function* func = new Function();
    func->line = peek().lineNumber;

    if (isType(peek().text)) {
        func->retType = get().text;
    } else {
        delete func;
        return nullptr;
    }

    if (peek().type == TokenType::IDENTIFIER) {
        func->name = get().text;
    } else {
        delete func;
        return nullptr;
    }

    if (!match(TokenType::PUNCTUATION, "(")) {
        delete func;
        return nullptr;
    }
     //parameter loop
    while (!check(TokenType::PUNCTUATION, ")") && !check(TokenType::END_OF_FILE)) {
        if (!isType(peek().text)) break;
        string paramType = get().text;
        string paramName = "";
        
        // Get parameter name 
        if (peek().type == TokenType::IDENTIFIER) {
            paramName = get().text;
            
            // Skip array brackets after parameter name , arr[]
            while (check(TokenType::PUNCTUATION, "[")) {
                get();  
                if (check(TokenType::PUNCTUATION, "]")) {
                    get();  
                } else {
                    break;
                }
            }
        }
        
        func->params.push_back({paramType, paramName});
        if (!match(TokenType::PUNCTUATION, ",")) break;
    }

    match(TokenType::PUNCTUATION, ")");
    func->body = (BlockStmt*)parseBlock();

    if (!func->body) {
        delete func;
        return nullptr;
    }

    return func;
}

vector<Function*> Parser::parse() {
    vector<Function*> funcs;

    while (peek().type != TokenType::END_OF_FILE) {
        if (check(TokenType::PUNCTUATION, "#")) {
            int curLine = peek().lineNumber;
            while (peek().lineNumber == curLine && peek().type != TokenType::END_OF_FILE) get();
            continue;
        }
        // detect function pattern
        if (isType(peek().text) && peek(1).type == TokenType::IDENTIFIER
            && peek(2).type == TokenType::PUNCTUATION && peek(2).text == "(") {
            Function* func = parseFunc();
            if (func && func->body) {
                funcs.push_back(func);
            } else if (func) {
                delete func;
            }
        } else {
            get();
        }
    }

    return funcs;
}

#endif