/*#ifndef CALLGRAPH_H
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

#endif*/
/*#ifndef CALLGRAPH_H
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
    
    void printLoopAnalysis() {
        auto sorted = getByEnergy();
        bool any = false;
        for (auto* n : sorted) {
            if (n->m.loops.empty() && !n->m.recInfo.active) continue;
            any = true;
            cout << "\nLOOP/RECURSION DETAIL  --  " << n->name << endl;
            cout << string(46, '-') << endl;
            int idx = 1;
            for (const auto& li : n->m.loops) {
                string ln = li.line > 0 ? " (line " + to_string(li.line) + ")" : "";
                cout << "  Loop #" << idx++ << "  [" << li.loopType << ln << "]" << endl;
                cout << "    Per-iteration energy : " << fixed << setprecision(4) << li.perIterEnergy << " units" << endl;
                cout << "    Iterations           :   x " << li.iterations << endl;
                cout << "    " << string(32, '-') << endl;
                cout << "    Loop subtotal        : " << fixed << setprecision(4) << li.totalLoopEnergy << " units";
                if (li.totalLoopEnergy >= n->directEnergy * 0.5) cout << "  <-- dominant";
                cout << "\n" << endl;
            }
            if (n->m.recInfo.active) {
                const RecInfo& ri = n->m.recInfo;
                cout << "  Recursion in : " << n->name << endl;
                cout << "    Per-call energy      : " << fixed << setprecision(4) << ri.perCallEnergy << " units" << endl;
                cout << "    Estimated depth      :   x " << ri.estimatedDepth << " calls" << endl;
                cout << "    " << string(32, '-') << endl;
                cout << "    Recursion total      : " << fixed << setprecision(4) << ri.totalRecEnergy << " units\n" << endl;
            }
        }
        if (!any) cout << "\n(no loops or recursion detected)\n" << endl;
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

#endif*/

#ifndef CALLGRAPH_H
#define CALLGRAPH_H

#include "Ast.h"
#include "AstVisitor.h"
#include "EnergyModel.h"
#include <bits/stdc++.h>
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
            n.name      = f->name;
            n.func      = f;
            n.callCount = 0;
            n.isRec     = false;
            n.recDepth  = 0;
            auto it = mmap.find(f->name);
            if (it != mmap.end()) n.m = it->second;
            nodes[f->name] = n;
        }

        for (auto* f : funcs) {
            CallDetector d;
            if (f->body) d.visitBlock(f->body);
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
        for (auto& p : nodes) res.push_back(&p.second);
        sort(res.begin(), res.end(),
             [](GraphNode* a, GraphNode* b) { return a->totalEnergy > b->totalEnergy; });
        return res;
    }

    void printSummary() {
        auto sorted = getByEnergy();
        cout << "\nENERGY SUMMARY" << endl;
        cout << "==============" << endl;
        cout << left  << setw(20) << "Function"
             << right << setw(10) << "Direct"
             << setw(10) << "Total"
             << setw(8)  << "Calls" << endl;
        cout << string(48, '-') << endl;
        for (auto* n : sorted) {
            cout << left  << setw(20) << n->name
                 << right << setw(10) << fixed << setprecision(2) << n->directEnergy
                 << setw(10) << n->totalEnergy
                 << setw(8)  << n->callCount << endl;
        }
        cout << endl;
    }

    // returns the highest-energy function excluding the given name (usually "main")
    GraphNode* getHotspot(const string& exclude = "main") {
        auto sorted = getByEnergy();
        for (auto* n : sorted) {
            if (n->name != exclude) return n;
        }
        return nullptr;
    }

    // prints summary table with hotspot highlighted
    void printSummaryWithHotspot(const string& hotspotName) {
        auto sorted = getByEnergy();
        cout << "\nENERGY SUMMARY" << endl;
        cout << "==============" << endl;
        cout << left  << setw(20) << "Function"
             << right << setw(10) << "Direct"
             << setw(13) << "Total"
             << setw(10)  << "Calls"
             << "  " << endl;
        cout << string(56, '-') << endl;
        for (auto* n : sorted) {
            bool isHot = (n->name == hotspotName);
            cout << left  << setw(20) << n->name
                 << right << setw(10) << fixed << setprecision(2) << n->directEnergy
                 << setw(13) << n->totalEnergy
                 << setw(10)  << n->callCount;
            if (isHot) cout << "  << HOTSPOT";
            cout << endl;
        }
        cout << endl;
    }

    void visualize(const string& root = "") {
        cout << "\nCALL GRAPH" << endl;
        cout << "==========" << endl;
        set<string> visited;
        if (!root.empty()) {
            printTree(root, 0, visited);
        } else {
            for (auto& p : nodes)
                if (p.second.callers.empty())
                    printTree(p.first, 0, visited);
        }
        cout << endl;
    }

    // detailed per-loop energy breakdown for a single named function
    void printHotspotBreakdown(const string& name) {
        auto* n = get(name);
        if (!n) { cout << "function not found: " << name << endl; return; }
        EnergyModel em;
        const Metrics& m = n->m;

        cout << "\nDETAILED BREAKDOWN: " << name << endl;
        cout << string(40, '=') << endl;

        double baseCost = (m.compCount  * em.compCost)
                        + (m.ioCount    * em.ioCost)
                        + (m.allocCount * em.allocCost)
                        + (m.loc        * em.locCost)
                        + (m.callCount  * em.callCost);

        cout << "\nBase (non-loop) operations:" << endl;
        if (m.compCount  > 0)
            cout << "  comparisons : " << m.compCount  << " x " << em.compCost
                 << " = " << fixed << setprecision(2) << m.compCount * em.compCost << endl;
        if (m.ioCount    > 0)
            cout << "  i/o calls   : " << m.ioCount    << " x " << em.ioCost
                 << " = " << m.ioCount * em.ioCost << endl;
        if (m.allocCount > 0)
            cout << "  allocations : " << m.allocCount << " x " << em.allocCost
                 << " = " << m.allocCount * em.allocCost << endl;
        if (m.callCount  > 0)
            cout << "  func calls  : " << m.callCount  << " x " << em.callCost
                 << " = " << m.callCount * em.callCost << endl;
        if (m.loc        > 0)
            cout << "  lines (loc) : " << m.loc        << " x " << em.locCost
                 << " = " << m.loc * em.locCost << endl;
        cout << "  base total  : " << fixed << setprecision(2) << baseCost << " units" << endl;

        if (m.loopDetails.empty()) {
            cout << "\n  no loops found." << endl;
        } else {
            cout << "\nLoop breakdown:" << endl;
            int loopNum = 1;
            double totalLoopCost = 0.0;
            for (const LoopInfo& li : m.loopDetails) {
                string indent(li.depth * 2, ' ');
                cout << "\n" << indent << "Loop " << loopNum++
                     << " [" << li.loopType << "]"
                     << "  depth=" << li.depth;
                if (li.line > 0) cout << "  line=" << li.line;
                cout << endl;

                cout << indent << "  estimated iterations : " << li.ownIters;
                if (li.ownIters == 10) cout << "  (variable bound, defaulted to 10)";
                cout << endl;

                cout << indent << "  operations per iteration:" << endl;
                if (li.arith > 0)
                    cout << indent << "    arithmetic  : " << li.arith << " ops x "
                         << em.arithCost << " = " << fixed << setprecision(2)
                         << li.arith * em.arithCost << endl;
                if (li.comp  > 0)
                    cout << indent << "    comparisons : " << li.comp << " ops x "
                         << em.compCost << " = " << li.comp * em.compCost << endl;
                if (li.logic > 0)
                    cout << indent << "    logical     : " << li.logic << " ops x "
                         << em.logicCost << " = " << li.logic * em.logicCost << endl;
                if (li.mem   > 0)
                    cout << indent << "    mem access  : " << li.mem << " ops x "
                         << em.arithCost << " = " << li.mem * em.arithCost << endl;
                if (li.io    > 0)
                    cout << indent << "    i/o         : " << li.io << " ops x "
                         << em.ioCost << " = " << li.io * em.ioCost << endl;
                if (li.alloc > 0)
                    cout << indent << "    allocations : " << li.alloc << " ops x "
                         << em.allocCost << " = " << li.alloc * em.allocCost << endl;
                if (li.calls > 0)
                    cout << indent << "    func calls  : " << li.calls << " ops x "
                         << em.callCost << " = " << li.calls * em.callCost << endl;

                cout << indent << "  energy per iteration : "
                     << fixed << setprecision(2) << li.perIterCost << " units" << endl;
                cout << indent << "  loop energy          : "
                     << li.perIterCost << " x " << li.ownIters
                     << " = " << fixed << setprecision(2) << li.loopCost << " units" << endl;
                totalLoopCost += li.loopCost;
            }
            cout << "\n  total loop cost     : " << fixed << setprecision(2)
                 << totalLoopCost << " units" << endl;
        }

        cout << "\n  direct energy       : " << fixed << setprecision(2)
             << n->directEnergy << " units" << endl;
        if (!n->callees.empty()) {
            double indirect = n->totalEnergy - n->directEnergy;
            cout << "  indirect (callees)  : " << fixed << setprecision(2)
                 << indirect << " units  (";
            for (size_t i = 0; i < n->callees.size(); i++) {
                cout << n->callees[i];
                if (i < n->callees.size()-1) cout << ", ";
            }
            cout << ")" << endl;
        }
        cout << "  total energy        : " << fixed << setprecision(2)
             << n->totalEnergy << " units" << endl;
        cout << endl;
    }

    // detailed per-loop energy breakdown for every function
    /*void printDetailedBreakdown() {
        auto sorted = getByEnergy();
        EnergyModel em;

        cout << "\nDETAILED ENERGY BREAKDOWN" << endl;
        cout << "=========================" << endl;

        for (auto* n : sorted) {
            const Metrics& m = n->m;

            cout << "\nFunction: " << n->name << endl;
            cout << string(40, '-') << endl;

            // non-loop base cost
            double baseCost = (m.compCount  * em.compCost)
                            + (m.ioCount    * em.ioCost)
                            + (m.allocCount * em.allocCost)
                            + (m.loc        * em.locCost)
                            + (m.callCount  * em.callCost);

            cout << "Base (non-loop) operations:" << endl;
            if (m.compCount  > 0)
                cout << "  comparisons : " << m.compCount  << " x " << em.compCost
                     << " = " << fixed << setprecision(2) << m.compCount * em.compCost << endl;
            if (m.ioCount    > 0)
                cout << "  i/o calls   : " << m.ioCount    << " x " << em.ioCost
                     << " = " << m.ioCount * em.ioCost << endl;
            if (m.allocCount > 0)
                cout << "  allocations : " << m.allocCount << " x " << em.allocCost
                     << " = " << m.allocCount * em.allocCost << endl;
            if (m.callCount  > 0)
                cout << "  func calls  : " << m.callCount  << " x " << em.callCost
                     << " = " << m.callCount * em.callCost << endl;
            if (m.loc        > 0)
                cout << "  lines (loc) : " << m.loc        << " x " << em.locCost
                     << " = " << m.loc * em.locCost << endl;
            cout << "  base total  : " << fixed << setprecision(2) << baseCost << " units" << endl;

            if (m.loopDetails.empty()) {
                cout << "  no loops found." << endl;
                cout << "  function energy: " << fixed << setprecision(2)
                     << n->directEnergy << " units" << endl;
                continue;
            }

            // per-loop breakdown
            cout << "\nLoop breakdown:" << endl;
            int loopNum = 1;
            for (const LoopInfo& li : m.loopDetails) {
                string indent(li.depth * 2, ' ');
                cout << "\n" << indent << "Loop " << loopNum++
                     << " [" << li.loopType << "]"
                     << " depth=" << li.depth;
                if (li.line > 0) cout << " line=" << li.line;
                cout << endl;

                cout << indent << "  estimated iterations : " << li.ownIters;
                if (li.ownIters == 10) cout << "  (variable bound, defaulted to 10)";
                cout << endl;

                cout << indent << "  operations per iteration:" << endl;
                double opCost = 0;
                if (li.arith > 0) {
                    double c = li.arith * em.arithCost;
                    cout << indent << "    arithmetic  : " << li.arith << " ops x "
                         << em.arithCost << " = " << fixed << setprecision(2) << c << endl;
                    opCost += c;
                }
                if (li.comp  > 0) {
                    double c = li.comp * em.compCost;
                    cout << indent << "    comparisons : " << li.comp << " ops x "
                         << em.compCost << " = " << c << endl;
                    opCost += c;
                }
                if (li.logic > 0) {
                    double c = li.logic * em.logicCost;
                    cout << indent << "    logical     : " << li.logic << " ops x "
                         << em.logicCost << " = " << c << endl;
                    opCost += c;
                }
                if (li.mem   > 0) {
                    double c = li.mem * em.arithCost;
                    cout << indent << "    mem access  : " << li.mem << " ops x "
                         << em.arithCost << " = " << c << endl;
                    opCost += c;
                }
                if (li.io    > 0) {
                    double c = li.io * em.ioCost;
                    cout << indent << "    i/o         : " << li.io << " ops x "
                         << em.ioCost << " = " << c << endl;
                    opCost += c;
                }
                if (li.alloc > 0) {
                    double c = li.alloc * em.allocCost;
                    cout << indent << "    allocations : " << li.alloc << " ops x "
                         << em.allocCost << " = " << c << endl;
                    opCost += c;
                }
                if (li.calls > 0) {
                    double c = li.calls * em.callCost;
                    cout << indent << "    func calls  : " << li.calls << " ops x "
                         << em.callCost << " = " << c << endl;
                    opCost += c;
                }
                cout << indent << "  energy per iteration : " << fixed << setprecision(2)
                     << li.perIterCost << " units" << endl;
                cout << indent << "  loop energy          : " << li.perIterCost
                     << " x " << li.ownIters << " = "
                     << fixed << setprecision(2) << li.loopCost << " units" << endl;
            }

            cout << "\n  total direct energy : " << fixed << setprecision(2)
                 << n->directEnergy << " units" << endl;
            cout << "  total energy (incl. callees) : " << fixed << setprecision(2)
                 << n->totalEnergy << " units" << endl;
        }
        cout << endl;
    }*/



    /*void printBreakdown(const string& name) {
        auto* n = get(name);
        if (!n) { cout << "function not found: " << name << endl; return; }

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
        if (n->callCount > 0)
            cout << "Call count: " << n->callCount << endl;
        if (n->isRec)
            cout << "Recursive (depth ~" << n->recDepth << ")" << endl;
    }*/

private:
    map<string, GraphNode> nodes;
    EnergyModel em;

    void detectRec() {
        for (auto& p : nodes) {
            set<string> v;
            if (hasRec(p.first, p.first, v, 0))
                p.second.isRec = true;
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
        for (const string& c : n->callees)
            if (hasRec(c, tgt, v, d + 1)) return true;
        return false;
    }

    void calcEnergy() {
        for (auto& p : nodes)
            p.second.directEnergy = em.compute(p.second.m);
        map<string, double> memo;
        for (auto& p : nodes)
            p.second.totalEnergy = calcTotal(p.first, memo);
    }

    double calcTotal(const string& name, map<string, double>& memo) {
        if (memo.find(name) != memo.end()) return memo[name];
        auto* n = get(name);
        if (!n) return 0.0;
        double tot = n->directEnergy;
        for (const string& c : n->callees)
            if (c != name) tot += calcTotal(c, memo);
        memo[name] = tot;
        return tot;
    }

    void printTree(const string& name, int depth, set<string>& visited) {
        for (int i = 0; i < depth; i++) cout << "  ";
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
        for (const string& c : n->callees)
            printTree(c, depth + 1, visited);
    }
};

#endif