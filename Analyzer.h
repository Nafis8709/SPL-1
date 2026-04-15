#ifndef ANALYZER_H
#define ANALYZER_H

#include "AST.h"
#include "ASTVisitor.h"
#include "EnergyModel.h"
#include<bits/stdc++.h>
using namespace std;

class EnergyAnalysisVisitor : public ASTVisitor {
public:
    EnergyAnalysisVisitor() : currentFunctionName(""), loopDepth(0) {}
    
    FunctionMetrics analyzeFunction(FunctionDefinition* func) {
        metrics = FunctionMetrics();
        currentFunctionName = func->name;
        loopDepth = 0;
        
        if (func->body) {
            visitFunctionDefinition(func);
        }
        
        return metrics;
    }
    
    void setAllFunctions(const vector<FunctionDefinition*>& functions) {
        allFunctionNames.clear();
        for (auto* func : functions) {
            allFunctionNames.insert(func->name);
        }
    }
    
    void visitBinaryExpression(BinaryExpression* expr) override {
        if (isArithmeticOp(expr->op)) {
            metrics.arithmeticCount++;
        } 
        else if (isComparisonOp(expr->op)) {
            metrics.comparisonCount++;
        } 
        else if (isLogicalOp(expr->op)) {
            metrics.logicalCount++;
        } 
        else if (isBitwiseOp(expr->op)) {
            metrics.bitwiseCount++;
        }
        
        ASTVisitor::visitBinaryExpression(expr);
    }
    
    void visitUnaryExpression(UnaryExpression* expr) override {
        if (expr->op == "*" || expr->op == "&") {
            metrics.memoryAccessCount++;
        } 
        else if (expr->op == "!") {
            metrics.logicalCount++;
        } 
        else if (expr->op == "~") {
            metrics.bitwiseCount++;
        } 
        else {
            metrics.arithmeticCount++;
        }
        
        ASTVisitor::visitUnaryExpression(expr);
    }
    
    void visitCallExpression(CallExpression* expr) override {
        metrics.functionCallCount++;
        
        if (isIOFunction(expr->functionName)) {
            metrics.inputOutputCount++;
        }
        
        if (isAllocationFunction(expr->functionName)) {
            metrics.allocationCount++;
        }
        
        if (expr->functionName == currentFunctionName) {
            metrics.recursionFlag = 1;
        }
        
        ASTVisitor::visitCallExpression(expr);
    }
    
    void visitArrayAccessExpression(ArrayAccessExpression* expr) override {
        metrics.memoryAccessCount++;
        ASTVisitor::visitArrayAccessExpression(expr);
    }
    
    void visitBlockStatement(BlockStatement* stmt) override {
        for (auto* s : stmt->statements) {
            visitStatement(s);
        }
    }
    
    void visitIfStatement(IfStatement* stmt) override {
        metrics.loc++;
        
        if (stmt->condition) {
            visitExpression(stmt->condition);
            metrics.comparisonCount++;
        }
        
        if (stmt->thenBranch) visitStatement(stmt->thenBranch);
        if (stmt->elseBranch) visitStatement(stmt->elseBranch);
    }
    
    void visitWhileStatement(WhileStatement* stmt) override {
        metrics.loc++;
        metrics.loopCount++;
        loopDepth++;
        
        long long iterations = 10;
        if (metrics.estimatediterationCount == 0) {
            metrics.estimatediterationCount = iterations;
        } else {
            metrics.estimatediterationCount *= iterations;
        }
        
        if (stmt->condition) visitExpression(stmt->condition);
        if (stmt->body) visitStatement(stmt->body);
        
        loopDepth--;
    }
    
    void visitForStatement(ForStatement* stmt) override {
        metrics.loc++;
        metrics.loopCount++;
        loopDepth++;
        
        long long iterations = estimateForLoopIterations(stmt);
        if (metrics.estimatediterationCount == 0) {
            metrics.estimatediterationCount = iterations;
        } else {
            metrics.estimatediterationCount *= iterations;
        }
        
        if (stmt->init) visitStatement(stmt->init);
        if (stmt->condition) visitExpression(stmt->condition);
        if (stmt->increment) visitExpression(stmt->increment);
        if (stmt->body) visitStatement(stmt->body);
        
        loopDepth--;
    }
    
    void visitReturnStatement(ReturnStatement* stmt) override {
        metrics.loc++;
        if (stmt->returnExpr) visitExpression(stmt->returnExpr);
    }
    
    void visitDeclarationStatement(DeclarationStatement* stmt) override {
        metrics.loc++;
        
        if (stmt->isArray) {
            metrics.allocationCount++;
        }
        
        if (stmt->initializer) visitExpression(stmt->initializer);
    }
    
    void visitExpressionStatement(ExpressionStatement* stmt) override {
        metrics.loc++;
        if (stmt->expr) visitExpression(stmt->expr);
    }
    
    void visitBreakStatement(BreakStatement* stmt) override {
        metrics.loc++;
    }
    
    void visitContinueStatement(ContinueStatement* stmt) override {
        metrics.loc++;
    }

private:
    FunctionMetrics metrics;
    string currentFunctionName;
    unordered_set<string> allFunctionNames;
    int loopDepth;
    
    long long estimateForLoopIterations(ForStatement* forStmt) {
        if (!forStmt->condition) return 10;
        
        if (forStmt->condition->type == ExpressionType::BINARY) {
            BinaryExpression* condExpr = (BinaryExpression*)forStmt->condition;
            
            if (condExpr->op == "<" || condExpr->op == "<=") {
                if (condExpr->right && condExpr->right->type == ExpressionType::NUMBER) {
                    NumberExpression* numExpr = (NumberExpression*)condExpr->right;
                    try {
                        long long bound = stoll(numExpr->number);
                        long long start = 0;
                        
                        if (forStmt->init && forStmt->init->type == StatementType::DECLARATION) {
                            DeclarationStatement* decl = (DeclarationStatement*)forStmt->init;
                            if (decl->initializer && decl->initializer->type == ExpressionType::NUMBER) {
                                NumberExpression* initNum = (NumberExpression*)decl->initializer;
                                start = stoll(initNum->number);
                            }
                        }
                        
                        long long iterations = bound - start;
                        if (condExpr->op == "<=") iterations++;
                        
                        return max(1LL, iterations);
                    } catch (...) {}
                }
            }
        }
        
        return 10;
    }
    
    bool isArithmeticOp(const string& op) {
        static const unordered_set<string> ops = {
            "+", "-", "*", "/", "%", "+=", "-=", "*=", "/=", "%=", "++", "--"
        };
        return ops.count(op) > 0;
    }
    
    bool isComparisonOp(const string& op) {
        static const unordered_set<string> ops = {
            "==", "!=", "<", ">", "<=", ">="
        };
        return ops.count(op) > 0;
    }
    
    bool isLogicalOp(const string& op) {
        static const unordered_set<string> ops = {"&&", "||", "!"};
        return ops.count(op) > 0;
    }
    
    bool isBitwiseOp(const string& op) {
        static const unordered_set<string> ops = {
            "&", "|", "^", "~", "<<", ">>", "&=", "|=", "^=", "<<=", ">>="
        };
        return ops.count(op) > 0;
    }
    
    bool isIOFunction(const string& funcName) {
        static const unordered_set<string> ioFuncs = {
            "printf", "scanf", "fprintf", "fscanf", "sprintf", "sscanf",
            "puts", "gets", "fgets", "fputs",
            "getchar", "putchar", "getc", "putc", "fgetc", "fputc",
            "fopen", "fclose", "fread", "fwrite", "fseek", "ftell",
            "cout", "cin", "cerr", "getline"
        };
        return ioFuncs.count(funcName) > 0;
    }
    
    bool isAllocationFunction(const string& funcName) {
        static const unordered_set<string> allocFuncs = {
            "malloc", "calloc", "realloc", "free", "new", "delete"
        };
        return allocFuncs.count(funcName) > 0;
    }
};

class ASTAnalyzer {
public:
    FunctionMetrics analyzeFunction(FunctionDefinition* func) {
        EnergyAnalysisVisitor visitor;
        visitor.setAllFunctions(allFunctions);
        return visitor.analyzeFunction(func);
    }
    
    void setAllFunctions(const vector<FunctionDefinition*>& functions) {
        allFunctions = functions;
    }

private:
    vector<FunctionDefinition*> allFunctions;
};

#endif