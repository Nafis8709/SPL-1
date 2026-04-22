/*#ifndef ANALYZER_H
#define ANALYZER_H

#include "Ast.h"
#include "AstVisitor.h"
#include "EnergyModel.h"
#include <bits/stdc++.h>
using namespace std;

class EnergyVisitor : public Visitor {
public:
    EnergyVisitor() : curFunc(""), loopDepth(0) {}

    Metrics analyze(Function* func) {
        m = Metrics();
        curFunc = func->name;
        loopDepth = 0;
        if (func->body) visitFunc(func);
        return m;
    }

    void setFuncs(const vector<Function*>& funcs) {
        funcNames.clear();
        for (auto* f : funcs) funcNames.insert(f->name);
    }

    void visitBinary(BinExpr* expr) override {
        if (isArith(expr->op)) m.arithCount++;
        else if (isCmp(expr->op)) m.compCount++;
        else if (isLogic(expr->op)) m.logicCount++;
        else if (isBit(expr->op)) m.bitCount++;
        Visitor::visitBinary(expr);
    }

    void visitUnary(UnaryExpr* expr) override {
        if (expr->op == "*" || expr->op == "&") m.memCount++;
        else if (expr->op == "!") m.logicCount++;
        else if (expr->op == "~") m.bitCount++;
        else m.arithCount++;
        Visitor::visitUnary(expr);
    }

    void visitCall(CallExpr* expr) override {
        m.callCount++;
        if (isIO(expr->funcName))    m.ioCount++;
        if (isAlloc(expr->funcName)) m.allocCount++;
        if (expr->funcName == curFunc) m.recursion = 1;
        Visitor::visitCall(expr);
    }

    void visitArray(ArrExpr* expr) override {
        m.memCount++;
        Visitor::visitArray(expr);
    }

    void visitBlock(BlockStmt* stmt) override {
        for (auto* s : stmt->statements) visitStmt(s);
    }

    void visitIf(IfStmt* stmt) override {
        m.loc++;
        if (stmt->cond) { visitExpr(stmt->cond); m.compCount++; }
        if (stmt->thenPart) visitStmt(stmt->thenPart);
        if (stmt->elsePart) visitStmt(stmt->elsePart);
    }

    void visitWhile(WhileStmt* stmt) override {
        m.loc++;
        m.loopCount++;
        loopDepth++;
        long long iters = 10;
        m.iterCount = (m.iterCount == 0) ? iters : m.iterCount * iters;
        if (stmt->cond) visitExpr(stmt->cond);
        if (stmt->body) visitStmt(stmt->body);
        loopDepth--;
    }

    void visitFor(ForStmt* stmt) override {
        m.loc++;
        m.loopCount++;
        loopDepth++;
        long long iters = estimateFor(stmt);
        m.iterCount = (m.iterCount == 0) ? iters : m.iterCount * iters;
        if (stmt->init) visitStmt(stmt->init);
        if (stmt->cond) visitExpr(stmt->cond);
        if (stmt->inc)  visitExpr(stmt->inc);
        if (stmt->body) visitStmt(stmt->body);
        loopDepth--;
    }

    void visitRet(RetStmt* stmt) override {
        m.loc++;
        if (stmt->retExpr) visitExpr(stmt->retExpr);
    }

    void visitDecl(DeclStmt* stmt) override {
        m.loc++;
        if (stmt->isArray) m.allocCount++;
        if (stmt->init) visitExpr(stmt->init);
    }

    void visitExprStmt(ExprStmt* stmt) override {
        m.loc++;
        if (stmt->expr) visitExpr(stmt->expr);
    }

    void visitBreak(BreakStmt* stmt) override    { m.loc++; }
    void visitContinue(ContinueStmt* stmt) override { m.loc++; }

private:
    Metrics m;
    string curFunc;
    unordered_set<string> funcNames;
    int loopDepth;

    long long estimateFor(ForStmt* s) {
        if (!s->cond) return 10;
        if (s->cond->type == ExprType::BINARY) {
            BinExpr* cond = (BinExpr*)s->cond;
            if (cond->op == "<" || cond->op == "<=") {
                if (cond->right && cond->right->type == ExprType::NUMBER) {
                    NumExpr* num = (NumExpr*)cond->right;
                    try {
                        long long bound = stoll(num->number);
                        long long start = 0;
                        if (s->init && s->init->type == StmtType::DECLARATION) {
                            DeclStmt* decl = (DeclStmt*)s->init;
                            if (decl->init && decl->init->type == ExprType::NUMBER) {
                                start = stoll(((NumExpr*)decl->init)->number);
                            }
                        }
                        long long iters = bound - start;
                        if (cond->op == "<=") iters++;
                        return max(1LL, iters);
                    } catch (...) {}
                }
            }
        }
        return 10;
    }

    bool isArith(const string& op) {
        static const unordered_set<string> s = {
            "+","-","*","/","%","+=","-=","*=","/=","%=","++","--"
        };
        return s.count(op);
    }
    bool isCmp(const string& op) {
        static const unordered_set<string> s = {"==","!=","<",">","<=",">="};
        return s.count(op);
    }
    bool isLogic(const string& op) {
        static const unordered_set<string> s = {"&&","||","!"};
        return s.count(op);
    }
    bool isBit(const string& op) {
        static const unordered_set<string> s = {
            "&","|","^","~","<<",">>","&=","|=","^=","<<=",">>="
        };
        return s.count(op);
    }
    bool isIO(const string& fn) {
        static const unordered_set<string> s = {
            "printf","scanf","fprintf","fscanf","sprintf","sscanf",
            "puts","gets","fgets","fputs","getchar","putchar",
            "getc","putc","fgetc","fputc","fopen","fclose",
            "fread","fwrite","fseek","ftell","cout","cin","cerr","getline"
        };
        return s.count(fn);
    }
    bool isAlloc(const string& fn) {
        static const unordered_set<string> s = {
            "malloc","calloc","realloc","free","new","delete"
        };
        return s.count(fn);
    }
};

class Analyzer {
public:
    Metrics analyze(Function* func) {
        EnergyVisitor v;
        v.setFuncs(allFuncs);
        Metrics m = v.analyze(func);
        m.energy = EnergyModel().compute(m);
        return m;
    }

    void setFuncs(const vector<Function*>& funcs) {
        allFuncs = funcs;
    }

private:
    vector<Function*> allFuncs;
};

#endif*/

/*#ifndef ANALYZER_H
#define ANALYZER_H

#include "Ast.h"
#include "AstVisitor.h"
#include "EnergyModel.h"
#include <bits/stdc++.h>
using namespace std;

class EnergyVisitor : public Visitor {
public:
    EnergyVisitor() : curFunc(""), loopDepth(0) {}

    Metrics analyze(Function* func) {
        m = Metrics();
        curFunc = func->name;
        loopDepth = 0;
        if (func->body) visitFunc(func);
        // populate recursion detail after full traversal
        if (m.recursion) {
            EnergyModel em;
            double base = (m.arithCount + m.logicCount + m.bitCount + m.memCount) * em.arithCost
                        + m.compCount * em.compCost + m.ioCount * em.ioCost
                        + m.allocCount * em.allocCost + m.loc * em.locCost
                        + m.callCount * em.callCost;
            m.recInfo.active         = true;
            m.recInfo.estimatedDepth = 10;
            m.recInfo.perCallEnergy  = base / 10.0;
            m.recInfo.totalRecEnergy = base;
        }
        return m;
    }

    void setFuncs(const vector<Function*>& funcs) {
        funcNames.clear();
        for (auto* f : funcs) funcNames.insert(f->name);
    }

    void visitBinary(BinExpr* expr) override {
        if (isArith(expr->op))       m.arithCount++;
        else if (isCmp(expr->op))    m.compCount++;
        else if (isLogic(expr->op))  m.logicCount++;
        else if (isBit(expr->op))    m.bitCount++;
        Visitor::visitBinary(expr);
    }

    void visitUnary(UnaryExpr* expr) override {
        if (expr->op == "*" || expr->op == "&") m.memCount++;
        else if (expr->op == "!")               m.logicCount++;
        else if (expr->op == "~")               m.bitCount++;
        else                                    m.arithCount++;
        Visitor::visitUnary(expr);
    }

    void visitCall(CallExpr* expr) override {
        m.callCount++;
        if (isIO(expr->funcName))    m.ioCount++;
        if (isAlloc(expr->funcName)) m.allocCount++;
        if (expr->funcName == curFunc) m.recursion = 1;
        Visitor::visitCall(expr);
    }

    void visitArray(ArrExpr* expr) override {
        m.memCount++;
        Visitor::visitArray(expr);
    }

    void visitBlock(BlockStmt* stmt) override {
        for (auto* s : stmt->statements) visitStmt(s);
    }

    void visitIf(IfStmt* stmt) override {
        m.loc++;
        if (stmt->cond) { visitExpr(stmt->cond); m.compCount++; }
        if (stmt->thenPart) visitStmt(stmt->thenPart);
        if (stmt->elsePart) visitStmt(stmt->elsePart);
    }

    void visitWhile(WhileStmt* stmt) override {
        m.loc++;
        m.loopCount++;
        loopDepth++;
        long long iters = estimateWhile(stmt);
        m.iterCount = (m.iterCount == 0) ? iters : m.iterCount * iters;
        // snapshot before body
        int a0=m.arithCount, c0=m.compCount, l0=m.logicCount, b0=m.bitCount, mm0=m.memCount;
        if (stmt->cond) visitExpr(stmt->cond);
        if (stmt->body) visitStmt(stmt->body);
        // per-iteration energy from delta
        double perIter = (m.arithCount-a0 + m.logicCount-l0 + m.bitCount-b0 + m.memCount-mm0) * 0.3
                       + (m.compCount-c0) * 0.5;
        LoopInfo li; li.loopType="while"; li.line=stmt->line;
        li.iterations=iters; li.perIterEnergy=perIter; li.totalLoopEnergy=perIter*iters;
        m.loops.push_back(li);
        loopDepth--;
    }

    void visitDoWhile(DoWhileStmt* stmt) override {
        m.loc++;
        m.loopCount++;
        loopDepth++;
        long long iters = estimateIterationsFromCond(stmt->cond);  // do-while condition checked same as while
        m.iterCount = (m.iterCount == 0) ? iters : m.iterCount * iters;
        // snapshot before body
        int a0=m.arithCount, c0=m.compCount, l0=m.logicCount, b0=m.bitCount, mm0=m.memCount;
        if (stmt->body) visitStmt(stmt->body);
        if (stmt->cond) visitExpr(stmt->cond);
        // per-iteration energy from delta
        double perIter = (m.arithCount-a0 + m.logicCount-l0 + m.bitCount-b0 + m.memCount-mm0) * 0.3
                       + (m.compCount-c0) * 0.5;
        LoopInfo li; li.loopType="do-while"; li.line=stmt->line;
        li.iterations=iters; li.perIterEnergy=perIter; li.totalLoopEnergy=perIter*iters;
        m.loops.push_back(li);
        loopDepth--;
    }

    void visitFor(ForStmt* stmt) override {
        m.loc++;
        m.loopCount++;
        loopDepth++;
        long long iters = estimateFor(stmt);
        m.iterCount = (m.iterCount == 0) ? iters : m.iterCount * iters;
        // snapshot before body
        int a0=m.arithCount, c0=m.compCount, l0=m.logicCount, b0=m.bitCount, mm0=m.memCount;
        if (stmt->init) visitStmt(stmt->init);
        if (stmt->cond) visitExpr(stmt->cond);
        if (stmt->inc)  visitExpr(stmt->inc);
        if (stmt->body) visitStmt(stmt->body);
        // per-iteration energy from delta
        double perIter = (m.arithCount-a0 + m.logicCount-l0 + m.bitCount-b0 + m.memCount-mm0) * 0.3
                       + (m.compCount-c0) * 0.5;
        LoopInfo li; li.loopType="for"; li.line=stmt->line;
        li.iterations=iters; li.perIterEnergy=perIter; li.totalLoopEnergy=perIter*iters;
        m.loops.push_back(li);
        loopDepth--;
    }

    void visitRet(RetStmt* stmt) override {
        m.loc++;
        if (stmt->retExpr) visitExpr(stmt->retExpr);
    }

    void visitDecl(DeclStmt* stmt) override {
        m.loc++;
        if (stmt->isArray) m.allocCount++;
        if (stmt->init) visitExpr(stmt->init);
    }

    void visitExprStmt(ExprStmt* stmt) override {
        m.loc++;
        if (stmt->expr) visitExpr(stmt->expr);
    }

    void visitBreak(BreakStmt* stmt) override    { m.loc++; }
    void visitContinue(ContinueStmt* stmt) override { m.loc++; }

private:
    Metrics m;
    string curFunc;
    unordered_set<string> funcNames;
    int loopDepth;

    long long estimateIterationsFromCond(Expr* cond) {
        if (!cond) return 10;
        if (cond->type == ExprType::BINARY) {
            BinExpr* bincond = (BinExpr*)cond;
            if (bincond->op == "<" || bincond->op == "<=") {
                // Try to extract bound from condition like: i < 100, i <= 50, etc.
                if (bincond->right && bincond->right->type == ExprType::NUMBER) {
                    NumExpr* num = (NumExpr*)bincond->right;
                    try {
                        long long bound = stoll(num->number);
                        return max(1LL, bound);
                    } catch (...) {}
                }
            }
        }
        return 10;
    }

    long long estimateWhile(WhileStmt* s) {
        if (!s->cond) return 10;
        return estimateIterationsFromCond(s->cond);
    }

    long long estimateFor(ForStmt* s) {
        if (!s->cond) return 10;
        if (s->cond->type == ExprType::BINARY) {
            BinExpr* cond = (BinExpr*)s->cond;
            if (cond->op == "<" || cond->op == "<=") {
                if (cond->right && cond->right->type == ExprType::NUMBER) {
                    NumExpr* num = (NumExpr*)cond->right;
                    try {
                        long long bound = stoll(num->number);
                        long long start = 0;
                        if (s->init && s->init->type == StmtType::DECLARATION) {
                            DeclStmt* decl = (DeclStmt*)s->init;
                            if (decl->init && decl->init->type == ExprType::NUMBER) {
                                start = stoll(((NumExpr*)decl->init)->number);
                            }
                        }
                        long long iters = bound - start;
                        if (cond->op == "<=") iters++;
                        return max(1LL, iters);
                    } catch (...) {}
                }
            }
        }
        return 10;
    }

    bool isArith(const string& op) {
        static const unordered_set<string> s = {
            "+","-","*","/","%","+=","-=","*=","/=","%=","++","--"
        };
        return s.count(op);
    }
    bool isCmp(const string& op) {
        static const unordered_set<string> s = {"==","!=","<",">","<=",">="};
        return s.count(op);
    }
    bool isLogic(const string& op) {
        static const unordered_set<string> s = {"&&","||","!"};
        return s.count(op);
    }
    bool isBit(const string& op) {
        static const unordered_set<string> s = {
            "&","|","^","~","<<",">>","&=","|=","^=","<<=",">>="
        };
        return s.count(op);
    }
    bool isIO(const string& fn) {
        static const unordered_set<string> s = {
            "printf","scanf","fprintf","fscanf","sprintf","sscanf",
            "puts","gets","fgets","fputs","getchar","putchar",
            "getc","putc","fgetc","fputc","fopen","fclose",
            "fread","fwrite","fseek","ftell","cout","cin","cerr","getline"
        };
        return s.count(fn);
    }
    bool isAlloc(const string& fn) {
        static const unordered_set<string> s = {
            "malloc","calloc","realloc","free","new","delete"
        };
        return s.count(fn);
    }
};

class Analyzer {
public:
    Metrics analyze(Function* func) {
        EnergyVisitor v;
        v.setFuncs(allFuncs);
        Metrics m = v.analyze(func);
        m.energy = EnergyModel().compute(m);
        return m;
    }

    void setFuncs(const vector<Function*>& funcs) {
        allFuncs = funcs;
    }

private:
    vector<Function*> allFuncs;
};

#endif*/

#ifndef ANALYZER_H
#define ANALYZER_H

#include "Ast.h"
#include "AstVisitor.h"
#include "EnergyModel.h"
#include <bits/stdc++.h>
using namespace std;

class EnergyVisitor : public Visitor {
public:
    EnergyVisitor() : curFunc(""), loopDepth(0) {}

    Metrics analyze(Function* func) {
        m = Metrics();
        curFunc = func->name;
        loopDepth = 0;
        if (func->body) visitFunc(func);
        return m;
    }

    void setFuncs(const vector<Function*>& funcs) {
        funcNames.clear();
        for (auto* f : funcs) funcNames.insert(f->name);
    }

    void visitBinary(BinExpr* expr) override {
        if      (isArith(expr->op))  m.arithCount++;
        else if (isCmp(expr->op))    m.compCount++;
        else if (isLogic(expr->op))  m.logicCount++;
        else if (isBit(expr->op))    m.bitCount++;
        Visitor::visitBinary(expr);
    }

    void visitUnary(UnaryExpr* expr) override {
        if      (expr->op=="*"||expr->op=="&") m.memCount++;
        else if (expr->op=="!")                m.logicCount++;
        else if (expr->op=="~")                m.bitCount++;
        else                                   m.arithCount++;
        Visitor::visitUnary(expr);
    }

    void visitCall(CallExpr* expr) override {
        m.callCount++;
        if (isIO(expr->funcName))    m.ioCount++;
        if (isAlloc(expr->funcName)) m.allocCount++;
        if (expr->funcName == curFunc) m.recursion = 1;
        Visitor::visitCall(expr);
    }

    void visitArray(ArrExpr* expr) override {
        m.memCount++;
        Visitor::visitArray(expr);
    }

    void visitBlock(BlockStmt* stmt) override {
        for (auto* s : stmt->statements) visitStmt(s);
    }

    void visitIf(IfStmt* stmt) override {
        m.loc++;
        if (stmt->cond) { visitExpr(stmt->cond); m.compCount++; }
        if (stmt->thenPart) visitStmt(stmt->thenPart);
        if (stmt->elsePart) visitStmt(stmt->elsePart);
    }

    void visitWhile(WhileStmt* stmt) override {
        m.loc++; m.loopCount++; loopDepth++;
        long long iters = 10;
        m.iterCount = (m.iterCount==0) ? iters : m.iterCount*iters;
        int a0=m.arithCount,c0=m.compCount,l0=m.logicCount;
        int io0=m.ioCount,b0=m.bitCount,mem0=m.memCount,al0=m.allocCount,ca0=m.callCount;
        if (stmt->cond) visitExpr(stmt->cond);
        if (stmt->body) visitStmt(stmt->body);
        recordLoop("while", stmt->line, iters, a0,c0,l0,io0,b0,mem0,al0,ca0);
        loopDepth--;
    }

    void visitFor(ForStmt* stmt) override {
        m.loc++; m.loopCount++; loopDepth++;
        long long iters = estimateFor(stmt);
        m.iterCount = (m.iterCount==0) ? iters : m.iterCount*iters;
        int a0=m.arithCount,c0=m.compCount,l0=m.logicCount;
        int io0=m.ioCount,b0=m.bitCount,mem0=m.memCount,al0=m.allocCount,ca0=m.callCount;
        if (stmt->init) visitStmt(stmt->init);
        if (stmt->cond) visitExpr(stmt->cond);
        if (stmt->inc)  visitExpr(stmt->inc);
        if (stmt->body) visitStmt(stmt->body);
        recordLoop("for", stmt->line, iters, a0,c0,l0,io0,b0,mem0,al0,ca0);
        loopDepth--;
    }

    void visitRet(RetStmt* stmt) override {
        m.loc++;
        if (stmt->retExpr) visitExpr(stmt->retExpr);
    }

    void visitDecl(DeclStmt* stmt) override {
        m.loc++;
        if (stmt->isArray) m.allocCount++;
        if (stmt->init) visitExpr(stmt->init);
    }

    void visitExprStmt(ExprStmt* stmt) override {
        m.loc++;
        if (stmt->expr) visitExpr(stmt->expr);
    }

    void visitBreak(BreakStmt* stmt)    override { m.loc++; }
    void visitContinue(ContinueStmt* s) override { m.loc++; }

private:
    Metrics m;
    string curFunc;
    unordered_set<string> funcNames;
    int loopDepth;

    void recordLoop(const string& type, int line, long long iters,
                    int a0,int c0,int l0,int io0,int b0,int mem0,int al0,int ca0) {
        LoopInfo li;
        li.loopType  = type;
        li.depth     = loopDepth;
        li.line      = line;
        li.ownIters  = iters;
        li.arith     = m.arithCount - a0;
        li.comp      = m.compCount  - c0;
        li.logic     = m.logicCount - l0;
        li.io        = m.ioCount    - io0;
        li.bit       = m.bitCount   - b0;
        li.mem       = m.memCount   - mem0;
        li.alloc     = m.allocCount - al0;
        li.calls     = m.callCount  - ca0;
        EnergyModel em;
        li.perIterCost = (li.arith+li.logic+li.bit+li.mem)*em.arithCost
                       + li.comp *em.compCost
                       + li.io   *em.ioCost
                       + li.alloc*em.allocCost
                       + li.calls*em.callCost;
        li.loopCost  = li.perIterCost * iters;
        m.loopDetails.push_back(li);
    }

    long long estimateFor(ForStmt* s) {
        if (!s->cond) return 10;
        if (s->cond->type == ExprType::BINARY) {
            BinExpr* cond = (BinExpr*)s->cond;
            if (cond->op=="<"||cond->op=="<=") {
                if (cond->right && cond->right->type==ExprType::NUMBER) {
                    try {
                        long long bound = stoll(((NumExpr*)cond->right)->number);
                        long long start = 0;
                        if (s->init && s->init->type==StmtType::DECLARATION) {
                            DeclStmt* d = (DeclStmt*)s->init;
                            if (d->init && d->init->type==ExprType::NUMBER)
                                start = stoll(((NumExpr*)d->init)->number);
                        }
                        long long it = bound-start;
                        if (cond->op=="<=") it++;
                        return max(1LL,it);
                    } catch(...) {}
                }
            }
        }
        return 10;
    }

    bool isArith(const string& op) {
        static const unordered_set<string> s={"+","-","*","/","%","+=","-=","*=","/=","%=","++","--"};
        return s.count(op);
    }
    bool isCmp(const string& op) {
        static const unordered_set<string> s={"==","!=","<",">","<=",">="};
        return s.count(op);
    }
    bool isLogic(const string& op) {
        static const unordered_set<string> s={"&&","||","!"};
        return s.count(op);
    }
    bool isBit(const string& op) {
        static const unordered_set<string> s={"&","|","^","~","<<",">>","&=","|=","^=","<<=",">>="};
        return s.count(op);
    }
    bool isIO(const string& fn) {
        static const unordered_set<string> s={
            "printf","scanf","fprintf","fscanf","sprintf","sscanf",
            "puts","gets","fgets","fputs","getchar","putchar",
            "getc","putc","fgetc","fputc","fopen","fclose",
            "fread","fwrite","fseek","ftell","cout","cin","cerr","getline"};
        return s.count(fn);
    }
    bool isAlloc(const string& fn) {
        static const unordered_set<string> s={"malloc","calloc","realloc","free","new","delete"};
        return s.count(fn);
    }
};

class Analyzer {
public:
    Metrics analyze(Function* func) {
        EnergyVisitor v;
        v.setFuncs(allFuncs);
        Metrics m = v.analyze(func);
        m.energy = EnergyModel().compute(m);
        return m;
    }
    void setFuncs(const vector<Function*>& funcs) { allFuncs = funcs; }
private:
    vector<Function*> allFuncs;
};

#endif

