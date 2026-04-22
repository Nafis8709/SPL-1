#ifndef CALLGRAPH_H
#define CALLGRAPH_H

#include "Ast.h"
#include "AstVisitor.h"
#include "EnergyModel.h"
#include<bits/stdc++.h>
using namespace std;

struct GraphNode {
    string name;
    Function* func;
    Metrics m;
    double directEnergy;
    double totalEnergy;
    vector<string> callees;
    vector<string> callers;
    int callCount;
    bool isRec;
    int recDepth;
};

class CallDetector : public Visitor {
public:
    set<string> calls;
    void visitCall(CallExpr* e) override {
        calls.insert(e->funcName);
        Visitor::visitCall(e);
    }
};

class CallGraph {
public:
    void build(vector<Function*>& funcs, const map<string, Metrics>& mmap) {
        nodes.clear();
        
        for (auto* f : funcs) {
            GraphNode n;
            n.name = f->name;
            n.func = f;
            n.callCount = 0;
            n.isRec = false;
            n.recDepth = 0;
            
            auto it = mmap.find(f->name);
            if (it != mmap.end()) {
                n.m = it->second;
            }
            nodes[f->name] = n;
        }
        for (auto* f : funcs) {
            CallDetector d;
            if (f->body) {
                d.visitBlock(f->body);
            }
            for (const string& c : d.calls) {
                if (nodes.find(c) != nodes.end()) {
                    nodes[f->name].callees.push_back(c);
                    nodes[c].callers.push_back(f->name);
                    nodes[c].callCount++;
                }
            }
        } 
        detectRec();
        calcEnergy();
    }
    GraphNode* get(const string& name) {
        auto it = nodes.find(name);
        return it != nodes.end() ? &it->second : nullptr;
    }
    vector<GraphNode*> getByEnergy() {
        vector<GraphNode*> res;
        for (auto& p : nodes) {
            res.push_back(&p.second);
        }
        sort(res.begin(), res.end(), 
             [](GraphNode* a, GraphNode* b) {
                 return a->totalEnergy > b->totalEnergy;
             });
        return res;
    }
    
    map<string, double> getBreakdown(const string& name) {
        map<string, double> bd;
        auto* n = get(name);
        if (!n) return bd;
        
        bd[name + " (direct)"] = n->directEnergy;
        for (const string& c : n->callees) {
            auto* cn = get(c);
            if (cn) bd[c] = cn->totalEnergy;
        }
        return bd;
    }
    
    void printSummary() {
        auto sorted = getByEnergy();
        
        cout << "\nENERGY SUMMARY" << endl;
        cout << "==============" << endl;
        cout << left << setw(20) << "Function" 
             << right << setw(10) << "Direct" 
             << setw(10) << "Total" 
             << setw(8) << "Calls" << endl;
        cout << string(48, '-') << endl;
        
        for (auto* n : sorted) {
            cout << left << setw(20) << n->name
                 << right << setw(10) << fixed << setprecision(2) << n->directEnergy
                 << setw(10) << n->totalEnergy
                 << setw(8) << n->callCount << endl;
        }
        cout << endl;
    }
    
    void printBreakdown(const string& name) {
        auto* n = get(name);
        if (!n) {
            cout << "function not found: " << name << endl;
            return;
        }
        cout << "\nBREAKDOWN: " << name << endl;
        cout << "==========" << endl;
        
        cout << "Direct: " << fixed << setprecision(2) << n->directEnergy << " units" << endl;
        
        if (!n->callees.empty()) {
            cout << "\nCalled functions:" << endl;
            double indirect = 0.0;
            for (const string& c : n->callees) {
                auto* cn = get(c);
                if (cn) {
                    cout << "  " << c << ": " << cn->totalEnergy << " units" << endl;
                    indirect += cn->totalEnergy;
                }
            }
            cout << "Indirect: " << indirect << " units" << endl;
        }
        
        cout << "Total: " << n->totalEnergy << " units\n" << endl;
        if (!n->callers.empty()) {
            cout << "Called by: ";
            for (size_t i = 0; i < n->callers.size(); i++) {
                cout << n->callers[i];
                if (i < n->callers.size() - 1) cout << ", ";
            }
            cout << endl;
        }
        
        if (n->callCount > 0) {
            cout << "Call count: " << n->callCount << endl;
        }
        
        if (n->isRec) {
            cout << "Recursive (depth ~" << n->recDepth << ")" << endl;
        }
    }
    
    void visualize(const string& root = "") {
        cout << "\nCALL GRAPH" << endl;
        cout << "==========" << endl;
        
        set<string> visited;
        if (!root.empty()) {
            printTree(root, 0, visited);
        } else {
            for (auto& p : nodes) {
                if (p.second.callers.empty()) {
                    printTree(p.first, 0, visited);
                }
            }
        }
        cout << endl;
    }

private:
    map<string, GraphNode> nodes;
    EnergyModel em;
    
    void detectRec() {
        for (auto& p : nodes) {
            set<string> v;
            if (hasRec(p.first, p.first, v, 0)) {
                p.second.isRec = true;
            }
        }
    }
    
    bool hasRec(const string& cur, const string& tgt, set<string>& v, int d) {
        if (d > 0 && cur == tgt) {
            nodes[tgt].recDepth = max(nodes[tgt].recDepth, d);
            return true;
        }
        if (v.count(cur)) return false;
        v.insert(cur);
        
        auto* n = get(cur);
        if (!n) return false;
        
        for (const string& c : n->callees) {
            if (hasRec(c, tgt, v, d + 1)) return true;
        }
        return false;
    }
    
    void calcEnergy() {
        for (auto& p : nodes) {
            p.second.directEnergy = em.compute(p.second.m);
        }
        
        map<string, double> memo;
        for (auto& p : nodes) {
            p.second.totalEnergy = calcTotal(p.first, memo);
        }
    }
    
    double calcTotal(const string& name, map<string, double>& memo) {
        if (memo.find(name) != memo.end()) {
            return memo[name];
        }
        
        auto* n = get(name);
        if (!n) return 0.0;
        
        double tot = n->directEnergy;
        for (const string& c : n->callees) {
            if (c != name) {
                tot += calcTotal(c, memo);
            }
        }
        
        memo[name] = tot;
        return tot;
    }
    
    void printTree(const string& name, int depth, set<string>& visited) {
        for (int i = 0; i < depth; i++) {
            cout << "  ";
        }
        
        auto* n = get(name);
        if (!n) return;
        
        cout << name << " [" << fixed << setprecision(2) << n->totalEnergy << "]";
        if (n->isRec) cout << " (rec)";
        cout << endl;
        
        if (visited.count(name)) {
            for (int i = 0; i <= depth; i++) cout << "  ";
            cout << "..." << endl;
            return;
        }
        
        visited.insert(name);
        for (const string& c : n->callees) {
            printTree(c, depth + 1, visited);
        }
    }
};

#endif