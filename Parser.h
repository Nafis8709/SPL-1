#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "AST.h"

class Parser {
public:
    Parser(const vector<Token>& toks) : tokens(toks), position(0) {}
    
    // Main parsing functions
    vector<FunctionDefinition*> parseFunctions();
    Expression* parseExpression();
    
private:
    const vector<Token>& tokens;
    int position;
    
    // Token manipulation
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

    // Expression parsing functions
    Expression* parsePrimary();
    Expression* parseUnary();
    Expression* parseBinaryRHS(int exprPrec, Expression* lhs);
    Expression* parsePostfix(Expression* expr);
    int getTokenPrecedence();
    
    // Statement parsing functions
    Statement* parseStatement();
    Statement* parseBlockStatement();
    Statement* parseIfStatement();
    Statement* parseWhileStatement();
    Statement* parseForStatement();
    Statement* parseReturnStatement();
    Statement* parseBreakStatement();
    Statement* parseContinueStatement();
    Statement* parseDeclarationStatement();
    Statement* parseExpressionStatement();
    
    // Function parsing
    FunctionDefinition* parseFunctionDefinition();
    
    // Helper functions
    bool isTypeKeyword(const string& word) const {
        return word == "int" || word == "long" || word == "short" || 
               word == "char" || word == "float" || word == "double" || 
               word == "void";
    }
};

// ============ EXPRESSION PARSING IMPLEMENTATION ============

Expression* Parser::parsePrimary() {
    Token tok = peek();
    
    // Handle identifiers and function calls
    if (tok.type == TokenType::IDENTIFIER) {
        string name = get().text;
        
        // Check for function call
        if (check(TokenType::PUNCTUATION, "(")) {
            get(); // consume '('
            CallExpression* call = new CallExpression(name);
            
            // Parse function arguments
            if (!check(TokenType::PUNCTUATION, ")")) {
                while (true) {
                    Expression* arg = parseExpression();
                    if (arg) call->arguments.push_back(arg);
                    if (!match(TokenType::PUNCTUATION, ",")) break;
                }
            }
            match(TokenType::PUNCTUATION, ")");
            return call;
        }
        
        return new IdentifierExpression(name);
    }
    
    // Handle numbers
    if (tok.type == TokenType::NUMBER) {
        return new NumberExpression(get().text);
    }
    
    // Handle strings
    if (tok.type == TokenType::STRING) {
        return new StringExpression(get().text);
    }
    
    // Handle parenthesized expressions
    if (match(TokenType::PUNCTUATION, "(")) {
        Expression* expr = parseExpression();
        match(TokenType::PUNCTUATION, ")");
        return expr;
    }
    
    return nullptr;
}

Expression* Parser::parsePostfix(Expression* expr) {
    // Handle array access: arr[index]
    while (check(TokenType::PUNCTUATION, "[")) {
        get(); // consume '['
        Expression* index = parseExpression();
        match(TokenType::PUNCTUATION, "]");
        expr = new ArrayAccessExpression(expr, index);
    }
    
    // Handle post-increment and post-decrement
    if (check(TokenType::OPERATOR, "++") || check(TokenType::OPERATOR, "--")) {
        string op = get().text;
        expr = new UnaryExpression(op, expr);
    }
    
    return expr;
}

Expression* Parser::parseUnary() {
    Token tok = peek();
    
    // Handle unary operators: +, -, !, ~, ++, --, *, &
    if (tok.type == TokenType::OPERATOR && 
        (tok.text == "+" || tok.text == "-" || tok.text == "!" || 
         tok.text == "~" || tok.text == "++" || tok.text == "--" || 
         tok.text == "*" || tok.text == "&")) {
        get();
        Expression* operand = parseUnary();
        if (!operand) return nullptr;
        return new UnaryExpression(tok.text, operand);
    }
    
    // Parse primary and handle postfix operations
    Expression* expr = parsePrimary();
    if (expr) {
        expr = parsePostfix(expr);
    }
    return expr;
}

int Parser::getTokenPrecedence() {
    Token tok = peek();
    if (tok.type != TokenType::OPERATOR) return -1;
    
    // Operator precedence table (higher number = higher precedence)
    static const unordered_map<string, int> precedences = {
        {"=", 1}, {"+=", 1}, {"-=", 1}, {"*=", 1}, {"/=", 1}, {"%=", 1},
        {"&=", 1}, {"|=", 1}, {"^=", 1}, {"<<=", 1}, {">>=", 1},
        {"||", 2},
        {"&&", 3},
        {"|", 4},
        {"^", 5},
        {"&", 6},
        {"==", 7}, {"!=", 7},
        {"<", 8}, {">", 8}, {"<=", 8}, {">=", 8},
        {"<<", 9}, {">>", 9},
        {"+", 10}, {"-", 10},
        {"*", 11}, {"/", 11}, {"%", 11}
    };
    
    auto it = precedences.find(tok.text);
    return it != precedences.end() ? it->second : -1;
}

Expression* Parser::parseBinaryRHS(int exprPrec, Expression* lhs) {
    while (true) {
        int tokPrec = getTokenPrecedence();
        if (tokPrec < exprPrec) return lhs;
        
        Token opTok = get();
        Expression* rhs = parseUnary();
        if (!rhs) {
            delete lhs;
            return nullptr;
        }
        
        int nextPrec = getTokenPrecedence();
        if (tokPrec < nextPrec) {
            rhs = parseBinaryRHS(tokPrec + 1, rhs);
            if (!rhs) {
                delete lhs;
                return nullptr;
            }
        }
        
        lhs = new BinaryExpression(lhs, opTok.text, rhs);
    }
}

Expression* Parser::parseExpression() {
    Expression* lhs = parseUnary();
    if (!lhs) return nullptr;
    return parseBinaryRHS(0, lhs);
}

// ============ STATEMENT PARSING IMPLEMENTATION ============

Statement* Parser::parseBlockStatement() {
    if (!match(TokenType::PUNCTUATION, "{")) return nullptr;
    
    BlockStatement* block = new BlockStatement();
    block->lineNumber = peek().lineNumber;
    
    // Parse all statements until we hit '}'
    while (!check(TokenType::PUNCTUATION, "}") && !check(TokenType::END_OF_FILE)) {
        Statement* stmt = parseStatement();
        if (stmt) {
            block->statements.push_back(stmt);
        }
    }
    
    match(TokenType::PUNCTUATION, "}");
    return block;
}

Statement* Parser::parseIfStatement() {
    int lineNum = peek().lineNumber;
    match(TokenType::KEYWORD, "if");
    match(TokenType::PUNCTUATION, "(");
    
    Expression* condition = parseExpression();
    
    match(TokenType::PUNCTUATION, ")");
    
    Statement* thenBranch = parseStatement();
    Statement* elseBranch = nullptr;
    
    // Parse optional else branch
    if (match(TokenType::KEYWORD, "else")) {
        elseBranch = parseStatement();
    }
    
    IfStatement* ifStmt = new IfStatement(condition, thenBranch, elseBranch);
    ifStmt->lineNumber = lineNum;
    return ifStmt;
}

Statement* Parser::parseWhileStatement() {
    int lineNum = peek().lineNumber;
    match(TokenType::KEYWORD, "while");
    match(TokenType::PUNCTUATION, "(");
    
    Expression* condition = parseExpression();
    
    match(TokenType::PUNCTUATION, ")");
    
    Statement* body = parseStatement();
    
    WhileStatement* whileStmt = new WhileStatement(condition, body);
    whileStmt->lineNumber = lineNum;
    return whileStmt;
}

Statement* Parser::parseForStatement() {
    int lineNum = peek().lineNumber;
    match(TokenType::KEYWORD, "for");
    match(TokenType::PUNCTUATION, "(");
    
    // Parse initialization (can be declaration or expression)
    Statement* init = nullptr;
    if (!check(TokenType::PUNCTUATION, ";")) {
        if (isTypeKeyword(peek().text)) {
            init = parseDeclarationStatement();
        } else {
            Expression* initExpr = parseExpression();
            match(TokenType::PUNCTUATION, ";");
            if (initExpr) {
                init = new ExpressionStatement(initExpr);
            }
        }
    } else {
        match(TokenType::PUNCTUATION, ";");
    }
    
    // Parse condition
    Expression* condition = nullptr;
    if (!check(TokenType::PUNCTUATION, ";")) {
        condition = parseExpression();
    }
    match(TokenType::PUNCTUATION, ";");
    
    // Parse increment
    Expression* increment = nullptr;
    if (!check(TokenType::PUNCTUATION, ")")) {
        increment = parseExpression();
    }
    match(TokenType::PUNCTUATION, ")");
    
    // Parse loop body
    Statement* body = parseStatement();
    
    ForStatement* forStmt = new ForStatement(init, condition, increment, body);
    forStmt->lineNumber = lineNum;
    return forStmt;
}

Statement* Parser::parseReturnStatement() {
    int lineNum = peek().lineNumber;
    match(TokenType::KEYWORD, "return");
    
    Expression* expr = nullptr;
    if (!check(TokenType::PUNCTUATION, ";")) {
        expr = parseExpression();
    }
    
    match(TokenType::PUNCTUATION, ";");
    
    ReturnStatement* retStmt = new ReturnStatement(expr);
    retStmt->lineNumber = lineNum;
    return retStmt;
}

Statement* Parser::parseBreakStatement() {
    int lineNum = peek().lineNumber;
    match(TokenType::KEYWORD, "break");
    match(TokenType::PUNCTUATION, ";");
    
    BreakStatement* breakStmt = new BreakStatement();
    breakStmt->lineNumber = lineNum;
    return breakStmt;
}

Statement* Parser::parseContinueStatement() {
    int lineNum = peek().lineNumber;
    match(TokenType::KEYWORD, "continue");
    match(TokenType::PUNCTUATION, ";");
    
    ContinueStatement* contStmt = new ContinueStatement();
    contStmt->lineNumber = lineNum;
    return contStmt;
}

Statement* Parser::parseDeclarationStatement() {
    int lineNum = peek().lineNumber;
    
    // Parse type
    string varType = get().text;
    
    // Parse variable name
    string varName = get().text;
    
    DeclarationStatement* decl = new DeclarationStatement(varType, varName);
    decl->lineNumber = lineNum;
    
    // Check for array declaration: int arr[10]
    if (check(TokenType::PUNCTUATION, "[")) {
        get(); // consume '['
        decl->isArray = true;
        
        if (peek().type == TokenType::NUMBER) {
            decl->arraySize = stoi(get().text);
        }
        
        match(TokenType::PUNCTUATION, "]");
    }
    
    // Parse optional initializer: int x = 5
    if (match(TokenType::OPERATOR, "=")) {
        decl->initializer = parseExpression();
    }
    
    match(TokenType::PUNCTUATION, ";");
    return decl;
}

Statement* Parser::parseExpressionStatement() {
    int lineNum = peek().lineNumber;
    Expression* expr = parseExpression();
    match(TokenType::PUNCTUATION, ";");
    
    if (!expr) return nullptr;
    
    ExpressionStatement* exprStmt = new ExpressionStatement(expr);
    exprStmt->lineNumber = lineNum;
    return exprStmt;
}

Statement* Parser::parseStatement() {
    // Block statement
    if (check(TokenType::PUNCTUATION, "{")) {
        return parseBlockStatement();
    }
    
    // Control flow statements
    if (check(TokenType::KEYWORD, "if")) {
        return parseIfStatement();
    }
    if (check(TokenType::KEYWORD, "while")) {
        return parseWhileStatement();
    }
    if (check(TokenType::KEYWORD, "for")) {
        return parseForStatement();
    }
    if (check(TokenType::KEYWORD, "return")) {
        return parseReturnStatement();
    }
    if (check(TokenType::KEYWORD, "break")) {
        return parseBreakStatement();
    }
    if (check(TokenType::KEYWORD, "continue")) {
        return parseContinueStatement();
    }
    
    // Declaration statement (type followed by identifier)
    if (isTypeKeyword(peek().text) && peek(1).type == TokenType::IDENTIFIER) {
        return parseDeclarationStatement();
    }
    
    // Expression statement (default)
    return parseExpressionStatement();
}

// ============ FUNCTION PARSING IMPLEMENTATION ============

FunctionDefinition* Parser::parseFunctionDefinition() {
    FunctionDefinition* func = new FunctionDefinition();
    func->lineNumber = peek().lineNumber;
    
    // Parse return type
    if (isTypeKeyword(peek().text)) {
        func->returnType = get().text;
    } else {
        return nullptr; // Invalid function
    }
    
    // Parse function name
    if (peek().type == TokenType::IDENTIFIER) {
        func->name = get().text;
    } else {
        delete func;
        return nullptr;
    }
    
    // Parse parameters
    if (!match(TokenType::PUNCTUATION, "(")) {
        delete func;
        return nullptr;
    }
    
    // Parse parameter list
    while (!check(TokenType::PUNCTUATION, ")") && !check(TokenType::END_OF_FILE)) {
        // Parse parameter type
        if (!isTypeKeyword(peek().text)) break;
        string paramType = get().text;
        
        // Handle array parameters: int arr[]
        if (check(TokenType::PUNCTUATION, "[")) {
            get(); // consume '['
            match(TokenType::PUNCTUATION, "]");
            paramType += "[]";
        }
        
        // Parse parameter name
        string paramName = "";
        if (peek().type == TokenType::IDENTIFIER) {
            paramName = get().text;
        }
        
        func->parameters.push_back(make_pair(paramType, paramName));
        
        if (!match(TokenType::PUNCTUATION, ",")) break;
    }
    
    match(TokenType::PUNCTUATION, ")");
    
    // Parse function body
    func->body = (BlockStatement*)parseBlockStatement();
    
    if (!func->body) {
        delete func;
        return nullptr;
    }
    
    return func;
}

vector<FunctionDefinition*> Parser::parseFunctions() {
    vector<FunctionDefinition*> functions;
    
    while (peek().type != TokenType::END_OF_FILE) {
        // Skip preprocessor directives (#include, #define, etc.)
        if (check(TokenType::PUNCTUATION, "#")) {
            int currentLine = peek().lineNumber;
            while (peek().lineNumber == currentLine && peek().type != TokenType::END_OF_FILE) {
                get();
            }
            continue;
        }
        
        // Try to parse a function
        if (isTypeKeyword(peek().text) && peek(1).type == TokenType::IDENTIFIER 
            && peek(2).type == TokenType::PUNCTUATION && peek(2).text == "(") {
            
            FunctionDefinition* func = parseFunctionDefinition();
            if (func && func->body) {
                functions.push_back(func);
            }
        } else {
            // Skip unknown tokens
            get();
        }
    }
    
    return functions;
}

#endif