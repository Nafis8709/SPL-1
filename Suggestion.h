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

/*#ifndef ADVISOR_H
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

#endif*/

#ifndef SUGGESTION_H
#define SUGGESTION_H

#include "Ast.h"
#include "CallGraph.h"
#include "EnergyModel.h"
#include <bits/stdc++.h>
using namespace std;

struct Suggestion {
    string func;
    int    priority = 0;   // 1=Critical 2=High 3=Medium 4=Low
    string category;
    string issue;
    string alternative;    // what to use instead
    string codeHint;       // concrete code snippet showing the alternative
    double saving  = 0.0;
    int    line    = 0;
};

class Advisor {
public:
    vector<Suggestion> analyze(const map<string, Metrics>& mmap, CallGraph& cg) {
        suggs.clear();
        for (const auto& p : mmap) {
            checkLoops  (p.first, p.second, cg);
            checkIO     (p.first, p.second, cg);
            checkRec    (p.first, p.second, cg);
            checkMem    (p.first, p.second, cg);
            checkComplex(p.first, p.second, cg);
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

        cout << "\nOPTIMIZATION SUGGESTIONS (HOTSPOT ANALYSIS)" << endl;
        cout << "============================================" << endl;

        map<int, string> labels = {{1,"CRITICAL"},{2,"HIGH"},{3,"MEDIUM"},{4,"LOW"}};

        int cnt = 1;
        for (const auto& s : suggs) {
            cout << "\n[" << cnt++ << "] " << labels[s.priority]
                 << " - " << s.category << endl;
            cout << "Function : " << s.func;
            if (s.line > 0) cout << " (line " << s.line << ")";
            cout << endl;
            cout << "Issue    : " << s.issue << endl;
            cout << "Instead  : " << s.alternative << endl;

            if (!s.codeHint.empty()) {
                cout << "Example  :" << endl;
                // indent each line of the code hint
                istringstream ss(s.codeHint);
                string line;
                while (getline(ss, line))
                    cout << "    " << line << endl;
            }

            //cout << "Saving   : ~" << fixed << setprecision(1) << s.saving << " units";
            //if (s.saving > 0 && totEnergy > 0) {
            //    double pct = (s.saving / totEnergy) * 100.0;
            //    cout << " (" << fixed << setprecision(1) << pct << "%)";
            //}
            cout << "\n" << string(50, '-') << endl;
        }
    }

    // prints only suggestions for a specific function (the hotspot)
    void printHotspotSuggestions(const string& funcName) {
        vector<Suggestion> hot;
        for (const auto& s : suggs)
            if (s.func == funcName) hot.push_back(s);

        if (hot.empty()) {
            cout << "\nno suggestions for " << funcName << endl;
            return;
        }

        map<int, string> labels = {{1,"CRITICAL"},{2,"HIGH"},{3,"MEDIUM"},{4,"LOW"}};

        cout << "\nOPTIMIZATION SUGGESTIONS FOR HOTSPOT: " << funcName << endl;
        cout << string(50, '=') << endl;

        // show potential saving headline
        double totSave = 0.0;
        for (const auto& s : hot) totSave += s.saving;
        cout << "Potential saving: " << fixed << setprecision(2) << totSave << " units";
        if (totEnergy > 0)
            cout << "  (" << fixed << setprecision(1)
                 << (totSave / totEnergy) * 100.0 << "% of total energy)";
        cout << endl;

        int cnt = 1;
        for (const auto& s : hot) {
            cout << "\n[" << cnt++ << "] " << labels[s.priority]
                 << " - " << s.category << endl;
            cout << "Issue    : " << s.issue << endl;
            cout << "Instead  : " << s.alternative << endl;
            if (!s.codeHint.empty()) {
                cout << "Example  :" << endl;
                istringstream ss(s.codeHint);
                string line;
                while (getline(ss, line))
                    cout << "    " << line << endl;
            }
           // cout << "Saving   : ~" << fixed << setprecision(1) << s.saving << " units";
           // if (s.saving > 0 && totEnergy > 0)
            //    cout << "  (" << fixed << setprecision(1)
              //       << (s.saving / totEnergy) * 100.0 << "%)";
            cout << "\n" << string(50, '-') << endl;
        }
    }

    void printSummary() {
        if (suggs.empty()) return;
        cout << "\nIMPACT SUMMARY" << endl;
        cout << "==============" << endl;

        double totSave = 0.0;
        map<string,int> catCnt;
        map<int,int>    priCnt;
        for (const auto& s : suggs) {
            totSave += s.saving;
            catCnt[s.category]++;
            priCnt[s.priority]++;
        }

        cout << "Total suggestions : " << suggs.size() << endl;
        cout << "Potential savings : " << fixed << setprecision(2) << totSave << " units" << endl;
        if (totEnergy > 0)
            cout << "Reduction         : " << fixed << setprecision(1)
                 << (totSave / totEnergy) * 100.0 << "%" << endl;

        cout << "\nBy priority:" << endl;
        if (priCnt[1] > 0) cout << "  Critical: " << priCnt[1] << endl;
        if (priCnt[2] > 0) cout << "  High:     " << priCnt[2] << endl;
        if (priCnt[3] > 0) cout << "  Medium:   " << priCnt[3] << endl;
        if (priCnt[4] > 0) cout << "  Low:      " << priCnt[4] << endl;

        cout << "\nBy category:" << endl;
        for (const auto& p : catCnt)
            cout << "  " << p.first << ": " << p.second << endl;
        cout << endl;
    }

    void setTotal(double e) { totEnergy = e; }

private:
    vector<Suggestion> suggs;
    double totEnergy = 0.0;

    // LOOP CHECKS
    void checkLoops(const string& fn, const Metrics& m, CallGraph& cg) {
        if (m.loopCount == 0) return;
        auto* n = cg.get(fn);
        if (!n) return;

        // high iteration nested loops (O(n^2) or worse pattern)
        if (m.loopCount >= 2 && m.iterCount >= 100) {
            Suggestion s;
            s.func     = fn;
            s.priority = 1;
            s.category = "Nested Loops";
            s.issue    = to_string(m.loopCount) + " nested loops with ~"
                       + to_string(m.iterCount) + " total iterations — O(n^k) complexity";
            //s.saving   = n->directEnergy * 0.4;

            // detect DP-table pattern: heavy memory access inside nested loops
            if (m.memCount > 15 && m.loopCount >= 2) {
                s.alternative = "Use an array instead of full 2D table "
                                "to cut memory and cache cost";
                s.codeHint =
                    "// Instead of: int table[m+1][n+1];\n"
                    "// Use two 1D arrays, swap each outer iteration:\n"
                    "int prev[n+1] = {0}, curr[n+1] = {0};\n"
                    "for (int i = 1; i <= m; i++) {\n"
                    "    for (int j = 1; j <= n; j++) {\n"
                    "        if (a[i-1] == b[j-1])\n"
                    "            curr[j] = prev[j-1] + 1;\n"
                    "        else\n"
                    "            curr[j] = max(prev[j], curr[j-1]);\n"
                    "    }\n"
                    "    memcpy(prev, curr, (n+1) * sizeof(int));\n"
                    "    memset(curr, 0,    (n+1) * sizeof(int));\n"
                    "}\n"
                    "// Reduces memory from O(m*n) to O(n), fewer cache misses";
            } else {
                s.alternative = "Reduce loop nesting - try hash maps or prefix arrays "
                                "to turn O(n^2) into O(n)";
                s.codeHint =
                    "// If searching for a match inside a nested loop:\n"
                    "// Instead of:\n"
                    "for (int i = 0; i < n; i++)\n"
                    "    for (int j = 0; j < n; j++)\n"
                    "        if (arr[i] == arr[j]) ...\n"
                    "\n"
                    "// Use an unordered_set for O(1) lookup:\n"
                    "unordered_set<int> seen;\n"
                    "for (int i = 0; i < n; i++) {\n"
                    "    if (seen.count(arr[i])) { /* match */ }\n"
                    "    seen.insert(arr[i]);\n"
                    "}";
            }
            suggs.push_back(s);
        }

        // single loop with very high iteration count
        if (m.loopCount == 1 && m.iterCount > 100) {
            Suggestion s;
            s.func      = fn;
            s.priority  = 2;
            s.category  = "Loop Optimization";
            s.issue     = "single loop with ~" + to_string(m.iterCount)
                        + " iterations — high iteration cost";
            s.alternative = "Add early-exit conditions or use binary search "
                            "if the data is sorted";
            s.codeHint  =
                "// Instead of scanning the whole array:\n"
                "for (int i = 0; i < n; i++)\n"
                "    if (arr[i] == target) return i;\n"
                "\n"
                "// If sorted, use binary search O(log n) vs O(n):\n"
                "int lo = 0, hi = n - 1;\n"
                "while (lo <= hi) {\n"
                "    int mid = (lo + hi) / 2;\n"
                "    if (arr[mid] == target) return mid;\n"
                "    else if (arr[mid] < target) lo = mid + 1;\n"
                "    else hi = mid - 1;\n"
                "}";
            s.saving    = n->directEnergy * 0.3;
            suggs.push_back(s);
        }
    }

    // I/O CHECKS
    void checkIO(const string& fn, const Metrics& m, CallGraph& cg) {
        if (m.ioCount == 0) return;
        auto* n = cg.get(fn);
        if (!n) return;

        // I/O present AND loops present in same function
        if (m.loopCount > 0 && m.ioCount > 0) {
            Suggestion s;
            s.func      = fn;
            s.priority  = 1;
            s.category  = "I/O in Loop";
            s.issue     = to_string(m.ioCount) + " I/O call(s) inside a function "
                        + "that also has " + to_string(m.loopCount) + " loop(s) — "
                        + "I/O is the most expensive operation (cost 1.0 per call)";
            s.saving    = m.ioCount * m.iterCount * 0.8;

            // reading char-by-char pattern (fgetc in a loop)
            if (m.ioCount >= 1 && m.loopCount >= 1) {
                s.alternative = "Replace character-by-character I/O with bulk fread — "
                                "reads the whole file in one syscall instead of one per character";
                s.codeHint  =
                    "// Instead of reading one char at a time:\n"
                    "char ch;\n"
                    "while ((ch = fgetc(file)) != EOF) { a[i++] = ch; }\n"
                    "\n"
                    "// Use fread to load the whole file at once:\n"
                    "char a[1024];\n"
                    "int n = fread(a, 1, sizeof(a) - 1, file);\n"
                    "a[n] = '\\0';\n"
                    "// One syscall instead of n syscalls — much cheaper";
            }
            suggs.push_back(s);
        }

        // many I/O calls, no loops
        else if (m.ioCount > 3) {
            Suggestion s;
            s.func      = fn;
            s.priority  = 3;
            s.category  = "Multiple I/O";
            s.issue     = to_string(m.ioCount) + " separate I/O calls";
            s.alternative = "Combine output into a single printf/puts call using "
                            "a formatted string buffer";
            s.codeHint  =
                "// Instead of multiple printfs:\n"
                "printf(\"name: %s\\n\", name);\n"
                "printf(\"score: %d\\n\", score);\n"
                "printf(\"rank: %d\\n\",  rank);\n"
                "\n"
                "// Build into one buffer and print once:\n"
                "char buf[256];\n"
                "int len = 0;\n"
                "len += sprintf(buf+len, \"name: %s\\n\",  name);\n"
                "len += sprintf(buf+len, \"score: %d\\n\", score);\n"
                "len += sprintf(buf+len, \"rank: %d\\n\",  rank);\n"
                "fwrite(buf, 1, len, stdout);";
            s.saving    = m.ioCount * 0.5;
            suggs.push_back(s);
        }
    }

    // RECURSION CHECKS
    void checkRec(const string& fn, const Metrics& m, CallGraph& cg) {
        if (!m.recursion) return;
        auto* n = cg.get(fn);
        if (!n) return;

        Suggestion s;
        s.func      = fn;
        s.priority  = 2;
        s.category  = "Recursion";
        s.issue     = fn + " calls itself recursively — each call adds a stack "
                    + "frame (alloc + overhead) and risks stack overflow on large input";
        s.saving    = n->directEnergy * 0.4;

        // detect factorial/fibonacci-like pattern ,few ops, self-call
        if (m.arithCount <= 5 && m.loopCount == 0) {
            s.alternative = "Convert to iterative with a simple loop — "
                            "eliminates all stack frame overhead";
            s.codeHint  =
                "// Recursive version (expensive):\n"
                "int factorial(int n) {\n"
                "    if (n <= 1) return 1;\n"
                "    return n * factorial(n - 1);\n"
                "}\n"
                "\n"
                "// Iterative version (cheap):\n"
                "int factorial(int n) {\n"
                "    int result = 1;\n"
                "    for (int i = 2; i <= n; i++)\n"
                "        result *= i;\n"
                "    return result;\n"
                "}";
        } else {
            // more complex recursion so memoization first 
            s.alternative = "Add memoization to avoid recomputing the same subproblems";
            s.codeHint  =
                "// Add a cache map before the recursive calls:\n"
                "unordered_map<int, int> memo;\n"
                "\n"
                "int solve(int n) {\n"
                "    if (memo.count(n)) return memo[n];  // already computed\n"
                "    if (n <= 1) return n;\n"
                "    int result = solve(n-1) + solve(n-2);\n"
                "    memo[n] = result;   // store before returning\n"
                "    return result;\n"
                "}";
        }
        suggs.push_back(s);
    }

    // MEMORY CHECKS
    void checkMem(const string& fn, const Metrics& m, CallGraph& cg) {
        auto* n = cg.get(fn);
        if (!n) return;

        // repeated array access inside loops ,high memCount with loops
        if (m.loopCount > 0 && m.memCount > 10) {
            Suggestion s;
            s.func      = fn;
            s.priority  = 2;
            s.category  = "Memory Access";
            s.issue     = to_string(m.memCount) + " array/pointer accesses — "
                        + "repeated indexing causes cache pressure inside loops";
            s.alternative = "Cache frequently accessed array elements in local variables "
                            "before the loop — local vars stay in CPU registers";
            s.codeHint  =
                "// Instead of indexing the array every iteration:\n"
                "for (int i = 0; i < n; i++)\n"
                "    total += arr[i] * arr[i];\n"
                "\n"
                "// Cache the value in a local variable:\n"
                "for (int i = 0; i < n; i++) {\n"
                "    int val = arr[i];   // one load, stays in register\n"
                "    total += val * val;\n"
                "}";
            s.saving    = n->directEnergy * 0.2;
            suggs.push_back(s);
        }

        // explicit dynamic allocations...malloc/calloc
        if (m.allocCount > 2) {
            Suggestion s;
            s.func      = fn;
            s.priority  = 3;
            s.category  = "Dynamic Allocation";
            s.issue     = to_string(m.allocCount) + " allocations detected — "
                        + "dynamic allocation costs 2.0 units each (OS involvement)";
            s.alternative = "Pre-allocate a single buffer before the loop and reuse it";
            s.codeHint  =
                "// Instead of allocating inside each iteration:\n"
                "for (int i = 0; i < n; i++) {\n"
                "    char* buf = malloc(256);  // expensive each time\n"
                "    process(buf);\n"
                "    free(buf);\n"
                "}\n"
                "\n"
                "// Allocate once outside, reuse:\n"
                "char buf[256];   // stack allocation, cost ~0\n"
                "for (int i = 0; i < n; i++) {\n"
                "    memset(buf, 0, 256);  // just clear it\n"
                "    process(buf);\n"
                "}";
            s.saving    = m.allocCount * 1.5;
            suggs.push_back(s);
        }
    }


    // COMPLEXITY CHECKS
    void checkComplex(const string& fn, const Metrics& m, CallGraph& cg) {
        auto* n = cg.get(fn);
        if (!n) return;

        int totalOps = m.arithCount + m.compCount + m.logicCount + m.bitCount;

        // high operation count — suggest lookup table
        if (totalOps > 40 && m.loopCount > 0) {
            Suggestion s;
            s.func      = fn;
            s.priority  = 3;
            s.category  = "High Complexity";
            s.issue     = to_string(totalOps) + " operations in a loop — "
                        + "repeated computation of the same values";
            s.alternative = "Use a precomputed lookup table for values that don't "
                            "change between iterations";
            s.codeHint  =
                "// If computing the same thing every iteration:\n"
                "for (int i = 0; i < n; i++)\n"
                "    result += expensive_formula(i);\n"
                "\n"
                "// Precompute once into an array:\n"
                "int table[MAX_N];\n"
                "for (int i = 0; i < MAX_N; i++)\n"
                "    table[i] = expensive_formula(i);\n"
                "\n"
                "// Then use the table:\n"
                "for (int i = 0; i < n; i++)\n"
                "    result += table[i];  // just an array read";
            s.saving    = n->directEnergy * 0.25;
            suggs.push_back(s);
        }

        // function is called very frequently by others
        if (n->callCount > 5 && n->directEnergy > 5.0) {
            Suggestion s;
            s.func      = fn;
            s.priority  = 3;
            s.category  = "Frequent Calls";
            s.issue     = fn + " is called " + to_string(n->callCount)
                        + " times — each call has stack frame overhead";
            s.alternative = "Inline small functions or use a result cache (memoization) "
                            "if the function is called with repeated inputs";
            s.codeHint  =
                "// For a small frequently-called function like max():\n"
                "// Instead of: result = max(a, b);  (function call overhead)\n"
                "\n"
                "// Use a macro or inline:\n"
                "#define MAX(a,b) ((a) > (b) ? (a) : (b))\n"
                "result = MAX(a, b);   // zero call overhead\n"
                "\n"
                "// Or in C++, use: inline int max(int a, int b) { return a>b?a:b; }";
            s.saving    = n->directEnergy * n->callCount * 0.15;
            suggs.push_back(s);
        }
    }
};

#endif