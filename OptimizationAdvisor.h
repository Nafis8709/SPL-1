#ifndef ADVISOR_H
#define ADVISOR_H

#include "AST.h"
#include "CallGraph.h"
#include "Energy.h"
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
    string functionName;
    int priority;           // 1=Critical, 2=High, 3=Medium, 4=Low
    string category;        // Loop, I/O, Recursion, Memory, etc.
    string issue;
    string suggestion;
    double potentialSaving; // Estimated energy savings (units)
    int lineNumber;
};

class OptimizationAdvisor {
public:
    // Analyze all functions and generate suggestions
    vector<OptimizationSuggestion> analyzeAndSuggest(
        const map<string, FunctionMetrics>& metricsMap,
        CallGraphAnalyzer& callGraph) {
        
        suggestions.clear();
        
        // Analyze each function
        for (const auto& pair : metricsMap) {
            const string& funcName = pair.first;
            const FunctionMetrics& metrics = pair.second;
            
            analyzeLoops(funcName, metrics, callGraph);
            analyzeIO(funcName, metrics, callGraph);
            analyzeRecursion(funcName, metrics, callGraph);
            analyzeMemory(funcName, metrics, callGraph);
            analyzeComplexity(funcName, metrics, callGraph);
        }
        
        // Sort by priority
        sort(suggestions.begin(), suggestions.end(),
             [](const OptimizationSuggestion& a, const OptimizationSuggestion& b) {
                 if (a.priority != b.priority) return a.priority < b.priority;
                 return a.potentialSaving > b.potentialSaving;
             });
        
        return suggestions;
    }
    
    // Print all suggestions
    void printSuggestions() {
        if (suggestions.empty()) {
            cout << "\n✓ No optimization suggestions - code is already efficient!\n" << endl;
            return;
        }
        
        cout << "\n╔════════════════════════════════════════════════════╗" << endl;
        cout << "║           OPTIMIZATION SUGGESTIONS                 ║" << endl;
        cout << "╚════════════════════════════════════════════════════╝\n" << endl;
        
        map<int, string> priorityLabels = {
            {1, "🔴 CRITICAL"},
            {2, "🟠 HIGH"},
            {3, "🟡 MEDIUM"},
            {4, "🟢 LOW"}
        };
        
        int count = 1;
        for (const auto& sugg : suggestions) {
            cout << "\n[" << count++ << "] " << priorityLabels[sugg.priority] 
                 << " - " << sugg.category << endl;
            cout << "Function: " << sugg.functionName;
            if (sugg.lineNumber > 0) {
                cout << " (Line " << sugg.lineNumber << ")";
            }
            cout << endl;
            
            cout << "\n📌 Issue:" << endl;
            cout << "   " << sugg.issue << endl;
            
            cout << "\n💡 Suggestion:" << endl;
            cout << "   " << sugg.suggestion << endl;
            
            cout << "\n💰 Potential Saving: ~" << fixed << setprecision(1) 
                 << sugg.potentialSaving << " units";
            
            if (sugg.potentialSaving > 0) {
                double percentage = (sugg.potentialSaving / getTotalEnergy()) * 100;
                cout << " (" << fixed << setprecision(1) << percentage << "%)";
            }
            cout << endl;
            
            cout << "────────────────────────────────────────────────────" << endl;
        }
    }
    
    // Print optimization summary
    void printSummary() {
        if (suggestions.empty()) return;
        
        cout << "\n╔════════════════════════════════════════════════════╗" << endl;
        cout << "║         OPTIMIZATION IMPACT SUMMARY                ║" << endl;
        cout << "╚════════════════════════════════════════════════════╝\n" << endl;
        
        double totalSavings = 0.0;
        map<string, int> categoryCount;
        map<int, int> priorityCount;
        
        for (const auto& sugg : suggestions) {
            totalSavings += sugg.potentialSaving;
            categoryCount[sugg.category]++;
            priorityCount[sugg.priority]++;
        }
        
        cout << "Total Suggestions:    " << suggestions.size() << endl;
        cout << "Potential Savings:    " << fixed << setprecision(2) 
             << totalSavings << " units" << endl;
        
        if (totalEnergy > 0) {
            double percentage = (totalSavings / totalEnergy) * 100;
            cout << "Percentage Reduction: " << fixed << setprecision(1) 
                 << percentage << "%" << endl;
        }
        
        cout << "\nBy Priority:" << endl;
        if (priorityCount[1] > 0) cout << "  Critical: " << priorityCount[1] << endl;
        if (priorityCount[2] > 0) cout << "  High:     " << priorityCount[2] << endl;
        if (priorityCount[3] > 0) cout << "  Medium:   " << priorityCount[3] << endl;
        if (priorityCount[4] > 0) cout << "  Low:      " << priorityCount[4] << endl;
        
        cout << "\nBy Category:" << endl;
        for (const auto& pair : categoryCount) {
            cout << "  " << pair.first << ": " << pair.second << endl;
        }
        cout << endl;
    }
    
    void setTotalEnergy(double energy) {
        totalEnergy = energy;
    }

private:
    vector<OptimizationSuggestion> suggestions;
    double totalEnergy = 0.0;
    
    double getTotalEnergy() const { return totalEnergy; }
    
    void analyzeLoops(const string& funcName, const FunctionMetrics& metrics,
                     CallGraphAnalyzer& callGraph) {
        if (metrics.loopCount == 0) return;
        
        auto* node = callGraph.getNode(funcName);
        if (!node) return;
        
        // High iteration count
        if (metrics.estimatediterationCount > 100) {
            OptimizationSuggestion sugg;
            sugg.functionName = funcName;
            sugg.priority = 1; // Critical
            sugg.category = "Loop Optimization";
            sugg.issue = "Loop with very high iteration count (" + 
                        to_string(metrics.estimatediterationCount) + " iterations)";
            sugg.suggestion = "Consider:\n" 
                            "   - Algorithm optimization (reduce complexity)\n"
                            "   - Early termination conditions\n"
                            "   - Processing data in batches";
            sugg.potentialSaving = node->directEnergy * 0.3; // 30% potential
            suggestions.push_back(sugg);
        }
        
        // Nested loops
        if (metrics.loopCount > 1 && metrics.estimatediterationCount > 10) {
            OptimizationSuggestion sugg;
            sugg.functionName = funcName;
            sugg.priority = 2; // High
            sugg.category = "Nested Loops";
            sugg.issue = "Multiple nested loops detected (" + 
                        to_string(metrics.loopCount) + " loops)";
            sugg.suggestion = "Consider:\n"
                            "   - Flatten nested loops if possible\n"
                            "   - Use more efficient data structures\n"
                            "   - Cache-friendly iteration patterns";
            sugg.potentialSaving = node->directEnergy * 0.25;
            suggestions.push_back(sugg);
        }
    }
    
    void analyzeIO(const string& funcName, const FunctionMetrics& metrics,
                  CallGraphAnalyzer& callGraph) {
        if (metrics.inputOutputCount == 0) return;
        
        auto* node = callGraph.getNode(funcName);
        if (!node) return;
        
        // I/O in loops
        if (metrics.loopCount > 0 && metrics.inputOutputCount > 0) {
            OptimizationSuggestion sugg;
            sugg.functionName = funcName;
            sugg.priority = 1; // Critical
            sugg.category = "I/O Optimization";
            sugg.issue = "I/O operations inside loops (" + 
                        to_string(metrics.inputOutputCount) + " I/O calls)";
            sugg.suggestion = "Consider:\n"
                            "   - Batch I/O operations outside loops\n"
                            "   - Use buffering techniques\n"
                            "   - Reduce frequency of I/O calls";
            sugg.potentialSaving = metrics.inputOutputCount * 
                                  metrics.estimatediterationCount * 0.8; // High saving
            suggestions.push_back(sugg);
        }
        
        // Multiple I/O operations
        else if (metrics.inputOutputCount > 3) {
            OptimizationSuggestion sugg;
            sugg.functionName = funcName;
            sugg.priority = 3; // Medium
            sugg.category = "I/O Optimization";
            sugg.issue = "Multiple I/O operations (" + 
                        to_string(metrics.inputOutputCount) + " calls)";
            sugg.suggestion = "Consider:\n"
                            "   - Combine multiple I/O into single operations\n"
                            "   - Use buffered I/O\n"
                            "   - Reduce I/O frequency where possible";
            sugg.potentialSaving = metrics.inputOutputCount * 0.5;
            suggestions.push_back(sugg);
        }
    }
    
    void analyzeRecursion(const string& funcName, const FunctionMetrics& metrics,
                         CallGraphAnalyzer& callGraph) {
        if (!metrics.recursionFlag) return;
        
        auto* node = callGraph.getNode(funcName);
        if (!node) return;
        
        OptimizationSuggestion sugg;
        sugg.functionName = funcName;
        sugg.priority = 2; // High
        sugg.category = "Recursion";
        sugg.issue = "Recursive implementation (estimated depth: 10)";
        sugg.suggestion = "Consider:\n"
                        "   - Convert to iterative approach\n"
                        "   - Use tail recursion if supported\n"
                        "   - Implement memoization/dynamic programming\n"
                        "   - Use iteration with explicit stack";
        sugg.potentialSaving = node->directEnergy * 0.4; // 40% potential
        suggestions.push_back(sugg);
    }
    
    void analyzeMemory(const string& funcName, const FunctionMetrics& metrics,
                      CallGraphAnalyzer& callGraph) {
        // Excessive memory allocations
        if (metrics.allocationCount > 2) {
            auto* node = callGraph.getNode(funcName);
            if (!node) return;
            
            OptimizationSuggestion sugg;
            sugg.functionName = funcName;
            sugg.priority = 3; // Medium
            sugg.category = "Memory Management";
            sugg.issue = "Multiple memory allocations (" + 
                        to_string(metrics.allocationCount) + " allocations)";
            sugg.suggestion = "Consider:\n"
                            "   - Reuse memory buffers\n"
                            "   - Allocate once, use multiple times\n"
                            "   - Use stack allocation where possible\n"
                            "   - Object pooling for frequent allocations";
            sugg.potentialSaving = metrics.allocationCount * 1.5;
            suggestions.push_back(sugg);
        }
        
        // Memory allocations in loops
        if (metrics.loopCount > 0 && metrics.allocationCount > 0) {
            auto* node = callGraph.getNode(funcName);
            if (!node) return;
            
            OptimizationSuggestion sugg;
            sugg.functionName = funcName;
            sugg.priority = 2; // High
            sugg.category = "Memory Management";
            sugg.issue = "Memory allocation inside loops";
            sugg.suggestion = "Consider:\n"
                            "   - Move allocations outside loops\n"
                            "   - Pre-allocate memory before loop\n"
                            "   - Use fixed-size buffers";
            sugg.potentialSaving = metrics.allocationCount * 
                                  metrics.estimatediterationCount * 1.8;
            suggestions.push_back(sugg);
        }
    }
    
    void analyzeComplexity(const string& funcName, const FunctionMetrics& metrics,
                          CallGraphAnalyzer& callGraph) {
        auto* node = callGraph.getNode(funcName);
        if (!node) return;
        
        // Very high operation count
        int totalOps = metrics.arithmeticCount + metrics.comparisonCount + 
                      metrics.logicalCount + metrics.bitwiseCount;
        
        if (totalOps > 50) {
            OptimizationSuggestion sugg;
            sugg.functionName = funcName;
            sugg.priority = 3; // Medium
            sugg.category = "Complexity";
            sugg.issue = "High operation count (" + to_string(totalOps) + " operations)";
            sugg.suggestion = "Consider:\n"
                            "   - Simplify algorithm if possible\n"
                            "   - Remove redundant calculations\n"
                            "   - Cache intermediate results\n"
                            "   - Use lookup tables for repeated calculations";
            sugg.potentialSaving = node->directEnergy * 0.2;
            suggestions.push_back(sugg);
        }
        
        // Function called frequently
        if (node->callCount > 10) {
            OptimizationSuggestion sugg;
            sugg.functionName = funcName;
            sugg.priority = 3; // Medium
            sugg.category = "Frequently Called";
            sugg.issue = "Function called " + to_string(node->callCount) + 
                        " times in the program";
            sugg.suggestion = "Consider:\n"
                            "   - Memoization for repeated inputs\n"
                            "   - Inline function if small and simple\n"
                            "   - Cache results of expensive computations\n"
                            "   - Reduce call frequency if possible";
            sugg.potentialSaving = node->directEnergy * node->callCount * 0.15;
            suggestions.push_back(sugg);
        }
    }
};

#endif