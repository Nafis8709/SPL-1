/*#ifndef ADVISOR_H
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

#endif*/

#ifndef ADVISOR_H
#define ADVISOR_H

#include "Ast.h"
#include "CallGraph.h"
#include "EnergyModel.h"
#include<bits/stdc++.h>
using namespace std;

struct Suggestion {
    string func;
    int priority = 4;
    string category;
    string hotspotDetail;
    string issue;
    string suggestion;
    double saving = 0.0;
    int line = 0;
};

class Advisor {
public:
    vector<Suggestion> analyze(const map<string, Metrics>& mmap, CallGraph& cg) {
        suggs.clear();
        
        // Build list of functions sorted by energy consumption (excluding main)
        vector<pair<string, double>> funcEnergy;
        for (const auto& p : mmap) {
            const string& fn = p.first;
            if (fn == "main") continue;  // Skip main - it's always a hotspot
            const Metrics& m = p.second;
            auto* n = cg.get(fn);
            double energy = n ? n->directEnergy : m.energy;
            funcEnergy.push_back({fn, energy});
        }
        
        // Sort by energy (descending) to find hotspots
        sort(funcEnergy.begin(), funcEnergy.end(),
             [](const pair<string, double>& a, const pair<string, double>& b) {
                 return a.second > b.second;
             });
        
        // Only analyze top hotspots (top 5 functions or energy > 10% of max)
        double maxEnergy = funcEnergy.empty() ? 0 : funcEnergy[0].second;
        double threshold = maxEnergy * 0.1;  // 10% of max energy
        int hotspotCount = 0;
        
        for (const auto& p : funcEnergy) {
            const string& fn = p.first;
            double energy = p.second;
            
            // Limit to top 5 hotspots or those above 10% threshold
            if (hotspotCount >= 5 && energy < threshold) break;
            
            auto it = mmap.find(fn);
            if (it != mmap.end()) {
                const Metrics& m = it->second;
                checkLoops(fn, m, cg);
                checkIO(fn, m, cg);
                checkRec(fn, m, cg);
                checkMem(fn, m, cg);
                checkComplex(fn, m, cg);
                hotspotCount++;
            }
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
        
        cout << "\nOPTIMIZATION HOTSPOTS  --  RANKED BY ENERGY IMPACT" << endl;
        cout << string(50, '=') << endl;
        
        map<int, string> labels = {{1,"CRITICAL"},{2,"HIGH"},{3,"MEDIUM"},{4,"LOW"}};
        int currentTier = 0;
        int cnt = 1;
        for (const auto& s : suggs) {
            if (s.priority != currentTier) {
                currentTier = s.priority;
                cout << "\n  --- TIER " << currentTier << " : " << labels[currentTier] << " ---" << endl;
            }
            cout << "\n[" << cnt++ << "] " << labels[s.priority] << " - " << s.category << endl;
            cout << "Function: " << s.func;
            if (s.line > 0) cout << " (line " << s.line << ")";
            cout << endl;
            if (!s.hotspotDetail.empty())
                cout << "Detail:   " << s.hotspotDetail << endl;
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
            // per-loop detail from LoopInfo if available
            if (!m.loops.empty()) {
                const auto& li = m.loops[0];
                ostringstream os;
                os << li.loopType << "-loop (line " << li.line << "): "
                   << fixed << setprecision(2) << li.perIterEnergy << " units/iter"
                   << " x " << li.iterations << " iters = "
                   << fixed << setprecision(2) << li.totalLoopEnergy << " units";
                s.hotspotDetail = os.str();
                s.issue = "Your " + li.loopType + "-loop on line " + to_string(li.line)
                        + " runs " + to_string(li.iterations) + " iterations at "
                        + to_string((int)(li.perIterEnergy*100)/100.0) + " units each"
                        + " -- top energy hotspot in '" + fn + "'";
            }
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
            s.hotspotDetail = to_string(m.loopCount) + " loops, combined "
                            + to_string(m.iterCount) + " total iterations";
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
        if (m.recInfo.active) {
            ostringstream os;
            os << fixed << setprecision(2) << m.recInfo.perCallEnergy
               << " units/call x ~" << m.recInfo.estimatedDepth
               << " calls = " << m.recInfo.totalRecEnergy << " units";
            s.hotspotDetail = os.str();
            s.issue = "recursive calls in '" + fn + "' stack ~"
                    + to_string(m.recInfo.estimatedDepth)
                    + " deep -- convert to iterative with memoization to avoid recomputation";
        }
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