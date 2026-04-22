#ifndef VISITOR_H
#define VISITOR_H

#include "AST.h"
#include<bits/stdc++.h>
using namespace std;

class Visitor {
public:
    virtual ~Visitor() {}
    
    virtual void visitIdent(IdentExpr* expr) {}
    virtual void visitNum(NumExpr* expr) {}
    virtual void visitStr(StrExpr* expr) {}
    
    virtual void visitUnary(UnaryExpr* expr) {
        if (expr->operand) visitExpr(expr->operand);
    }
    
    virtual void visitBinary(BinExpr* expr) {
        if (expr->left) visitExpr(expr->left);
        if (expr->right) visitExpr(expr->right);
    }
    
    virtual void visitCall(CallExpr* expr) {
        for (auto* arg : expr->args) {
            visitExpr(arg);
        }
    }
    
    virtual void visitArray(ArrExpr* expr) {
        if (expr->arrayName) visitExpr(expr->arrayName);
        if (expr->index) visitExpr(expr->index);
    }
    
    virtual void visitExpr(Expr* expr) {
        if (!expr) return;
        
    switch (expr->type) {
        case ExprType::IDENTIFIER:
            visitIdent((IdentExpr*)expr);
            break;
        case ExprType::NUMBER:
            visitNum((NumExpr*)expr);
            break;
        case ExprType::STRING:
            visitStr((StrExpr*)expr);
            break;
        case ExprType::UNARY:
            visitUnary((UnaryExpr*)expr);
            break;
        case ExprType::BINARY:
            visitBinary((BinExpr*)expr);
            break;
        case ExprType::CALL:
            visitCall((CallExpr*)expr);
            break;
        case ExprType::ARRAY_ACCESS:
            visitArray((ArrExpr*)expr);
            break;
        }
    }
    
    virtual void visitExprStmt(ExprStmt* stmt) {
        if (stmt->expr) visitExpr(stmt->expr);
    }
    
    virtual void visitDecl(DeclStmt* stmt) {
        if (stmt->init) visitExpr(stmt->init);
    }
    
    virtual void visitRet(RetStmt* stmt) {
        if (stmt->retExpr) visitExpr(stmt->retExpr);
    }
    
    virtual void visitBlock(BlockStmt* stmt) {
        for (auto* s : stmt->statements) {
            visitStmt(s);
        }
    }
    
    virtual void visitIf(IfStmt* stmt) {
        if (stmt->cond) visitExpr(stmt->cond);
        if (stmt->thenPart) visitStmt(stmt->thenPart);
        if (stmt->elsePart) visitStmt(stmt->elsePart);
    }
    
    virtual void visitWhile(WhileStmt* stmt) {
        if (stmt->cond) visitExpr(stmt->cond);
        if (stmt->body) visitStmt(stmt->body);
    }
    
    virtual void visitFor(ForStmt* stmt) {
        if (stmt->init) visitStmt(stmt->init);
        if (stmt->cond) visitExpr(stmt->cond);
        if (stmt->inc) visitExpr(stmt->inc);
        if (stmt->body) visitStmt(stmt->body);
    }
    
    virtual void visitBreak(BreakStmt* stmt) {}
    virtual void visitContinue(ContinueStmt* stmt) {}
    
    virtual void visitStmt(Stmt* stmt) {
        if (!stmt) return;
        
        switch (stmt->type) {
            case StmtType::EXPRESSION:
                visitExprStmt((ExprStmt*)stmt);
                break;
            case StmtType::DECLARATION:
                visitDecl((DeclStmt*)stmt);
                break;
            case StmtType::RETURN:
                visitRet((RetStmt*)stmt);
                break;
            case StmtType::BLOCK:
                visitBlock((BlockStmt*)stmt);
                break;
            case StmtType::IF:
                visitIf((IfStmt*)stmt);
                break;
            case StmtType::WHILE:
                visitWhile((WhileStmt*)stmt);
                break;
            case StmtType::FOR:
                visitFor((ForStmt*)stmt);
                break;
            case StmtType::BREAK:
                visitBreak((BreakStmt*)stmt);
                break;
            case StmtType::CONTINUE:
                visitContinue((ContinueStmt*)stmt);
                break;
        }
    }
    
    virtual void visitFunc(Function* func) {
        if (func->body) visitBlock(func->body);
    }
};

#endif