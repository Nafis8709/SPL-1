#ifndef Analyzer.h
#define Analyzer_h

#include "Ast.h"
#include "EnergyModel.h"

#include<bits/stdc++.h>
using namespace std;


class ASTAnalyzer {
public:
    
    FunctionMetrics analyzeFunction(FunctionDefinition* func) {
        FunctionMetrics metrics;
        currentFunctionName = func->functionName;
        allFunctionNames.clear();
        
        
        if (func->body) {
            analyzeStatement(func->body, metrics);
        }
        
        return metrics;
    }
    
    
    void setAllFunctions(const vector<FunctionDefinition*>& functions) {
        allFunctionNames.clear();
        for (auto* func : functions) {
            allFunctionNames.insert(func->functionName);
        }
    }

private:
    string currentFunctionName;
    unordered_set<string> allFunctionNames;
    
    void analyzeStatement(Statement* stmt, FunctionMetrics& metrics) {
        if (!stmt) return;
        
        metrics.loc++;
        
        switch (stmt->type) {
            case StatementType::BLOCK_STATEMENT: {
                BlockStatement* block = (BlockStatement*)stmt;
                
                metrics.loc--;
                for (auto* s : block->statements) {
                    analyzeStatement(s, metrics);
                }
                break;
            }
            
            case StatementType::IF_STATEMENT: {
                IfStatement* ifStmt = (IfStatement*)stmt;
                
                analyzeExpression(ifStmt->condition, metrics);
                metrics.comparisonCount++; 
                
                analyzeStatement(ifStmt->thenBranch, metrics);
                
                if (ifStmt->elseBranch) {
                    analyzeStatement(ifStmt->elseBranch, metrics);
                }
                break;
            }
            
            case StatementType::WHILE_STATEMENT: {
                WhileStatement* whileStmt = (WhileStatement*)stmt;
                metrics.loopCount++;
                
                analyzeExpression(whileStmt->condition, metrics);
                
                if (metrics.estimatediterationCount == 0) {
                    metrics.estimatediterationCount = 10;
                } else {
                    metrics.estimatediterationCount *= 10;
                }
        
                analyzeStatement(whileStmt->body, metrics);
                break;
            }
            
            case StatementType::FOR_STATEMENT: {
                ForStatement* forStmt = (ForStatement*)stmt;
                metrics.loopCount++;
                
                analyzeStatement(forStmt->initialization, metrics);
                
                analyzeExpression(forStmt->condition, metrics);
                
                analyzeStatement(forStmt->increment, metrics);
                
                long long iterations = estimateForLoopIterations(forStmt);
                if (metrics.estimatediterationCount == 0) {
                    metrics.estimatediterationCount = iterations;
                } else {
                    
                    metrics.estimatediterationCount *= iterations;
                }
                
                analyzeStatement(forStmt->body, metrics);
                break;
            }
            
            case StatementType::RETURN_STATEMENT: {
                ReturnStatement* retStmt = (ReturnStatement*)stmt;
                analyzeExpression(retStmt->returnValue, metrics);
                break;
            }
            
            case StatementType::DECLARATION_STATEMENT: {
                DeclarationStatement* decl = (DeclarationStatement*)stmt;
                
                if (decl->isArray) {
                    metrics.allocationCount++;
                }
                
                analyzeExpression(decl->initialValue, metrics);
                break;
            }
            
            case StatementType::EXPRESSION_STATEMENT: {
                ExpressionStatement* exprStmt = (ExpressionStatement*)stmt;
                analyzeExpression(exprStmt->expression, metrics);
                break;
            }
            
            case StatementType::BREAK_STATEMENT:
            case StatementType::CONTINUE_STATEMENT:
                
                break;
        }
    }
    
    
    void analyzeExpression(Expression* expr, FunctionMetrics& metrics) {
        if (!expr) return;
        
        switch (expr->type) {
            case ExpressionType::BINARY_OP: {
                BinaryOpExpression* binExpr = (BinaryOpExpression*)expr;
                
                
                if (isArithmeticOp(binExpr->op)) {
                    metrics.arithmeticCount++;
                } 
                else if (isComparisonOp(binExpr->op)) {
                    metrics.comparisonCount++;
                } 
                else if (isLogicalOp(binExpr->op)) {
                    metrics.logicalCount++;
                } 
                else if (isBitwiseOp(binExpr->op)) {
                    metrics.bitwiseCount++;
                }
                
                
                analyzeExpression(binExpr->left, metrics);
                analyzeExpression(binExpr->right, metrics);
                break;
            }
            
            case ExpressionType::UNARY_OP: {
                UnaryOpExpression* unExpr = (UnaryOpExpression*)expr;
                
                
                if (unExpr->op == "*" || unExpr->op == "&") {
                    metrics.memoryAccessCount++;
                } 
                
                else if (unExpr->op == "!") {
                    metrics.logicalCount++;
                } 
                
                else if (unExpr->op == "~") {
                    metrics.bitwiseCount++;
                } 
                
                else {
                    metrics.arithmeticCount++;
                }
                
                
                analyzeExpression(unExpr->operand, metrics);
                break;
            }
            
            case ExpressionType::FUNCTION_CALL: {
                FunctionCallExpression* callExpr = (FunctionCallExpression*)expr;
                metrics.functionCallCount++;
                
                
                if (isIOFunction(callExpr->functionName)) {
                    metrics.inputOutputCount++;
                }
                
                
                if (isAllocationFunction(callExpr->functionName)) {
                    metrics.allocationCount++;
                }
                
                
                if (callExpr->functionName == currentFunctionName) {
                    metrics.recursionFlag = 1;
                }
                
                
                for (auto* arg : callExpr->arguments) {
                    analyzeExpression(arg, metrics);
                }
                break;
            }
            
            case ExpressionType::ARRAY_ACCESS: {
                ArrayAccessExpression* arrExpr = (ArrayAccessExpression*)expr;
                
                metrics.memoryAccessCount++;
                
                
                analyzeExpression(arrExpr->arrayName, metrics);
                analyzeExpression(arrExpr->index, metrics);
                break;
            }
            
            case ExpressionType::IDENTIFIER:
            case ExpressionType::NUMBER:
            case ExpressionType::STRING:
                
                break;
        }
    }
    
    
    long long estimateForLoopIterations(ForStatement* forStmt) {
        
        
        
        if (!forStmt->condition) return 10; 
        
        
        if (forStmt->condition->type == ExpressionType::BINARY_OP) {
            BinaryOpExpression* condExpr = (BinaryOpExpression*)forStmt->condition;
            
            
            if (condExpr->op == "<" || condExpr->op == "<=") {
                if (condExpr->right && condExpr->right->type == ExpressionType::NUMBER) {
                    NumberExpression* numExpr = (NumberExpression*)condExpr->right;
                    try {
                        long long bound = stoll(numExpr->number);
                        
                        
                        long long start = 0;
                        if (forStmt->initialization && forStmt->initialization->type == StatementType::DECLARATION_STATEMENT) {
                            DeclarationStatement* decl = (DeclarationStatement*)forStmt->initialization;
                            if (decl->initialValue && decl->initialValue->type == ExpressionType::NUMBER) {
                                NumberExpression* initNum = (NumberExpression*)decl->initialValue;
                                start = stoll(initNum->number);
                            }
                        }
                        
                        long long iterations = bound - start;
                        if (condExpr->op == "<=") iterations++;
                        
                        return max(1LL, iterations); 
                    } catch (...) {
                        
                    }
                }
            }
        }
        
        return 10; 
    }
    
    
    bool isArithmeticOp(const string& op) {
        static const unordered_set<string> arithmeticOps = {
            "+", "-", "*", "/", "%",
            "+=", "-=", "*=", "/=", "%=",
            "++", "--"
        };
        return arithmeticOps.count(op) > 0;
    }
    
    
    bool isComparisonOp(const string& op) {
        static const unordered_set<string> comparisonOps = {
            "==", "!=", "<", ">", "<=", ">="
        };
        return comparisonOps.count(op) > 0;
    }
    
    
    bool isLogicalOp(const string& op) {
        static const unordered_set<string> logicalOps = {
            "&&", "||", "!"
        };
        return logicalOps.count(op) > 0;
    }
    
    
    bool isBitwiseOp(const string& op) {
        static const unordered_set<string> bitwiseOps = {
            "&", "|", "^", "~", "<<", ">>",
            "&=", "|=", "^=", "<<=", ">>="
        };
        return bitwiseOps.count(op) > 0;
    }
    
    
    bool isIOFunction(const string& funcName) {
        static const unordered_set<string> ioFunctions = {
            "printf", "scanf", "fprintf", "fscanf", "sprintf", "sscanf",
            "puts", "gets", "fgets", "fputs",
            "getchar", "putchar", "getc", "putc", "fgetc", "fputc",
            "fopen", "fclose", "fread", "fwrite", "fseek", "ftell",
            "cout", "cin", "cerr", "getline"
        };
        return ioFunctions.count(funcName) > 0;
    }
    
    
    bool isAllocationFunction(const string& funcName) {
        static const unordered_set<string> allocFunctions = {
            "malloc", "calloc", "realloc", "free",
            "new", "delete"
        };
        return allocFunctions.count(funcName) > 0;
    }
};

#endif