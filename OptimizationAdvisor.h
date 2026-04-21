#ifndef ADVISOR_H
#define ADVISOR_H

#include "Ast.h"
#include "CallGraph.h"
#include "EnergyModel.h"
#include<bits/stdc++.h>
using namespace std;

struct Suggestion {
    string func;
    int priority;
    string category;
    string issue;
    string suggestion;
    double saving;
    int line;
};

class Advisor {
public:
    vector<Suggestion> analyze(const map<string, Metrics>& mmap, CallGraph& cg) {
        suggs.clear();
        
        for (const auto& p : mmap) {
            const string& fn = p.first;
            const Metrics& m = p.second;
            
            checkLoops(fn, m, cg);
            checkIO(fn, m, cg);
            checkRec(fn, m, cg);
            checkMem(fn, m, cg);
            checkComplex(fn, m, cg);
        }
        
        sort(suggs.begin(), suggs.end(),
             [](const Suggestion& a, const Suggestion& b) {
                 if (a.priority != b.priority) return a.priority < b.priority;
                 return a.saving > b.saving;
             });
        
        return suggs;
    }
    
    void print() {
        if (suggs.empty()) {
            cout << "\nno optimization suggestions" << endl;
            return;
        }
        
        cout << "\nOPTIMIZATIONS" << endl;
        cout << "=============" << endl;
        
        map<int, string> labels = {
            {1, "CRITICAL"}, {2, "HIGH"}, {3, "MEDIUM"}, {4, "LOW"}
        };
        
        int cnt = 1;
        for (const auto& s : suggs) {
            cout << "\n[" << cnt++ << "] " << labels[s.priority] << " - " << s.category << endl;
            cout << "Function: " << s.func;
            if (s.line > 0) cout << " (line " << s.line << ")";
            cout << endl;
            
            cout << "Issue: " << s.issue << endl;
            cout << "Fix: " << s.suggestion << endl;
            cout << "Saving: ~" << fixed << setprecision(1) << s.saving << " units";
            
            if (s.saving > 0 && totEnergy > 0) {
                double pct = (s.saving / totEnergy) * 100;
                cout << " (" << fixed << setprecision(1) << pct << "%)";
            }
            cout << "\n" << string(50, '-') << endl;
        }
    }
    
    void printSummary() {
        if (suggs.empty()) return;
        
        cout << "\nIMPACT SUMMARY" << endl;
        cout << "==============" << endl;
        
        double totSave = 0.0;
        map<string, int> catCnt;
        map<int, int> priCnt;
        
        for (const auto& s : suggs) {
            totSave += s.saving;
            catCnt[s.category]++;
            priCnt[s.priority]++;
        }
        
        cout << "Total suggestions: " << suggs.size() << endl;
        cout << "Potential savings: " << fixed << setprecision(2) << totSave << " units" << endl;
        
        if (totEnergy > 0) {
            double pct = (totSave / totEnergy) * 100;
            cout << "Reduction: " << fixed << setprecision(1) << pct << "%" << endl;
        }
        
        cout << "\nBy priority:" << endl;
        if (priCnt[1] > 0) cout << "  Critical: " << priCnt[1] << endl;
        if (priCnt[2] > 0) cout << "  High: " << priCnt[2] << endl;
        if (priCnt[3] > 0) cout << "  Medium: " << priCnt[3] << endl;
        if (priCnt[4] > 0) cout << "  Low: " << priCnt[4] << endl;
        
        cout << "\nBy category:" << endl;
        for (const auto& p : catCnt) {
            cout << "  " << p.first << ": " << p.second << endl;
        }
        cout << endl;
    }
    
    void setTotal(double e) { totEnergy = e; }

private:
    vector<Suggestion> suggs;
    double totEnergy = 0.0;
    
    void checkLoops(const string& fn, const Metrics& m, CallGraph& cg) {
        if (m.loopCount == 0) return;
        auto* n = cg.get(fn);
        if (!n) return;
        
        if (m.iterCount > 100) {
            Suggestion s;
            s.func = fn;
            s.priority = 1;
            s.category = "Loop";
            s.issue = "high iteration count (" + to_string(m.iterCount) + ")";
            s.suggestion = "reduce complexity or add early exit";
            s.saving = n->directEnergy * 0.3;
            suggs.push_back(s);
        }
        
        if (m.loopCount > 1 && m.iterCount > 10) {
            Suggestion s;
            s.func = fn;
            s.priority = 2;
            s.category = "Nested loops";
            s.issue = to_string(m.loopCount) + " nested loops";
            s.suggestion = "flatten loops or use better data structures";
            s.saving = n->directEnergy * 0.25;
            suggs.push_back(s);
        }
    }
    
    void checkIO(const string& fn, const Metrics& m, CallGraph& cg) {
        if (m.ioCount == 0) return;
        auto* n = cg.get(fn);
        if (!n) return;
        
        if (m.loopCount > 0 && m.ioCount > 0) {
            Suggestion s;
            s.func = fn;
            s.priority = 1;
            s.category = "I/O";
            s.issue = "I/O inside loops";
            s.suggestion = "batch I/O outside loops";
            s.saving = m.ioCount * m.iterCount * 0.8;
            suggs.push_back(s);
        } else if (m.ioCount > 3) {
            Suggestion s;
            s.func = fn;
            s.priority = 3;
            s.category = "I/O";
            s.issue = "multiple I/O calls";
            s.suggestion = "combine I/O operations";
            s.saving = m.ioCount * 0.5;
            suggs.push_back(s);
        }
    }
    
    void checkRec(const string& fn, const Metrics& m, CallGraph& cg) {
        if (!m.recursion) return;
        auto* n = cg.get(fn);
        if (!n) return;
        
        Suggestion s;
        s.func = fn;
        s.priority = 2;
        s.category = "Recursion";
        s.issue = "recursive implementation";
        s.suggestion = "convert to iterative";
        s.saving = n->directEnergy * 0.4;
        suggs.push_back(s);
    }
    
    void checkMem(const string& fn, const Metrics& m, CallGraph& cg) {
        if (m.allocCount > 2) {
            auto* n = cg.get(fn);
            if (!n) return;
            
            Suggestion s;
            s.func = fn;
            s.priority = 3;
            s.category = "Memory";
            s.issue = "multiple allocations";
            s.suggestion = "reuse buffers";
            s.saving = m.allocCount * 1.5;
            suggs.push_back(s);
        }
        
        if (m.loopCount > 0 && m.allocCount > 0) {
            auto* n = cg.get(fn);
            if (!n) return;
            
            Suggestion s;
            s.func = fn;
            s.priority = 2;
            s.category = "Memory";
            s.issue = "allocation in loops";
            s.suggestion = "move allocation outside loops";
            s.saving = m.allocCount * m.iterCount * 1.8;
            suggs.push_back(s);
        }
    }
    
    void checkComplex(const string& fn, const Metrics& m, CallGraph& cg) {
        auto* n = cg.get(fn);
        if (!n) return;
        
        int totOps = m.arithCount + m.compCount + m.logicCount + m.bitCount;
        
        if (totOps > 50) {
            Suggestion s;
            s.func = fn;
            s.priority = 3;
            s.category = "Complexity";
            s.issue = "high operation count";
            s.suggestion = "simplify algorithm";
            s.saving = n->directEnergy * 0.2;
            suggs.push_back(s);
        }
        
        if (n->callCount > 10) {
            Suggestion s;
            s.func = fn;
            s.priority = 3;
            s.category = "Frequent calls";
            s.issue = "called " + to_string(n->callCount) + " times";
            s.suggestion = "consider caching or inlining";
            s.saving = n->directEnergy * n->callCount * 0.15;
            suggs.push_back(s);
        }
    }
};

#endif