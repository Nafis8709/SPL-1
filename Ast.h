#ifndef AST_H
#define AST_H

#include <bits/stdc++.h>
using namespace std;

struct FunctionDefinition {
    string returnType;
    string functionName;
    vector<pair<string, string>> parameters; 
    BlockStatement* body;

    int lineNumber=0;
    FunctionDefinition() : body(nullptr) {}
};

enum class ExpressionType{
    IDENTIFIER, 
    NUMBER, 
    STRING, 
    UNARY_OP, 
    BINARY_OP, 
    FUNCTION_CALL, 
    INDEXING, 
    ARRAY_ACCESS
};

struct Expression {
    ExpressionType type;
    string value; 
    vector<Expression> children; 
};

struct IdentifierExpression : Expression{
    string name;
    IdentifierExpression(const string& name) : name(name) {
        type = ExpressionType::IDENTIFIER;
        value = name;
    }
};

struct NumberExpression : Expression{
    string number;
    NumberExpression(const string& number) : number(number) {
        type = ExpressionType::NUMBER;
        value = number;
    }
};

struct StringExpression : Expression{
    string str;
    StringExpression(const string& str) : str(str) {
        type = ExpressionType::STRING;
        value = str;
    }
};

struct UnaryOpExpression : Expression{
    string op;
    Expression* operand;
    UnaryOpExpression(const string& op, Expression* operand) : op(op), operand(operand) {
        type = ExpressionType::UNARY_OP;
        value = op;
        children.push_back(*operand);
    }
};

struct BinaryOpExpression : Expression{
    string op;
    Expression* left;
    Expression* right;
    BinaryOpExpression(const string& op, Expression* left, Expression* right) : op(op), left(left), right(right) {
        type = ExpressionType::BINARY_OP;
        value = op;
        children.push_back(*left);
        children.push_back(*right);
    }
};

struct FunctionCallExpression : Expression{
    string functionName;
    vector<Expression*> arguments;
    FunctionCallExpression(const string& functionName, const vector<Expression*>& arguments) : functionName(functionName), arguments(arguments) {
        type = ExpressionType::FUNCTION_CALL;
        value = functionName;
        for(auto arg : arguments){
            children.push_back(*arg);
        }
    }
};

struct IndexingExpression : Expression{
    Expression* array;
    Expression* index;
    IndexingExpression(Expression* array, Expression* index) : array(array), index(index) {
        type = ExpressionType::INDEXING;
        value = "[]";
        children.push_back(*array);
        children.push_back(*index);
    }
};

struct ArrayAccessExpression : Expression{
    Expression* arrayName;
    Expression* index;
    ArrayAccessExpression(Expression* arrayName, Expression* index) : arrayName(arrayName), index(index) {
        type = ExpressionType::ARRAY_ACCESS;
        children.push_back(*arrayName);
        children.push_back(*index);
    }
};

enum class StatementType{
    EXPRESSION_STATEMENT, 
    DECLARATION_STATEMENT, 
    ASSIGNMENT_STATEMENT, 
    RETURN_STATEMENT, 
    IF_STATEMENT, 
    WHILE_STATEMENT, 
    FOR_STATEMENT, 
    BREAK_STATEMENT, 
    CONTINUE_STATEMENT,
    BLOCK_STATEMENT
};

struct Statement {
    StatementType type;
    int lineNumber=0;
    vector<Statement*> children;
};

struct ExpressionStatement : Statement{
    Expression* expression;
    ExpressionStatement(Expression* expression) : expression(expression) {
        type = StatementType::EXPRESSION_STATEMENT;
    }
};

struct DeclarationStatement : Statement{
    string varType;
    string varName;
    Expression* initialValue;
    int arraySize=-1;
    bool isArray=false;
    DeclarationStatement(const string& varType, const string& varName, Expression* initialValue = nullptr) 
        : varType(varType), varName(varName), initialValue(initialValue) {
        type = StatementType::DECLARATION_STATEMENT;
    }
};

struct ReturnStatement : Statement{
    Expression* returnValue;
    ReturnStatement(Expression* returnValue = nullptr) : returnValue(returnValue) {
        type = StatementType::RETURN_STATEMENT;
    }
};

struct IfStatement : Statement{
    Expression* condition;
    Statement* thenBranch;
    Statement* elseBranch;
    IfStatement(Expression* condition, Statement* thenBranch, Statement* elseBranch = nullptr) 
        : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {
        type = StatementType::IF_STATEMENT;
    }
};

struct WhileStatement : Statement{
    Expression* condition;
    Statement* body;
    WhileStatement(Expression* condition, Statement* body) : condition(condition), body(body) {
        type = StatementType::WHILE_STATEMENT;
    }
};

struct ForStatement : Statement{
    Statement* initialization;
    Expression* condition;
    Statement* increment;
    Statement* body;
    ForStatement(Statement* initialization, Expression* condition, Statement* increment, Statement* body) 
        : initialization(initialization), condition(condition), increment(increment), body(body) {
        type = StatementType::FOR_STATEMENT;
    }
};

struct BreakStatement : Statement{
    BreakStatement() {
        type = StatementType::BREAK_STATEMENT;
    }
};

struct ContinueStatement : Statement{
    ContinueStatement() {
        type = StatementType::CONTINUE_STATEMENT;
    }
};

struct BlockStatement : Statement{
    vector<Statement*> statements;
    BlockStatement() {
        type = StatementType::BLOCK_STATEMENT;
    }
};

#endif 



