#ifndef CALLGRAPH_H
#define CALLGRAPH_H

#include "AST.h"
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
    string functionName;
    FunctionDefinition* definition;
    FunctionMetrics metrics;
    double directEnergy;        // Energy of this function alone
    double totalEnergy;         // Energy including all called functions
    
    vector<string> callees;     // Functions this function calls
    vector<string> callers;     // Functions that call this function
    
    int callCount;              // How many times this function is called
    bool isRecursive;
    int maxRecursionDepth;
};

// Visitor to detect function calls
class CallDetectorVisitor : public ASTVisitor {
public:
    set<string> calledFunctions;
    
    void visitCallExpression(CallExpression* expr) override {
        calledFunctions.insert(expr->functionName);
        ASTVisitor::visitCallExpression(expr);
    }
};

class CallGraphAnalyzer {
public:
    // Build the call graph from all functions
    void buildCallGraph(vector<FunctionDefinition*>& functions, 
                       const map<string, FunctionMetrics>& metricsMap) {
        nodes.clear();
        
        // Step 1: Create nodes for all functions
        for (auto* func : functions) {
            CallGraphNode node;
            node.functionName = func->name;
            node.definition = func;
            node.callCount = 0;
            node.isRecursive = false;
            node.maxRecursionDepth = 0;
            
            // Get metrics for this function
            auto it = metricsMap.find(func->name);
            if (it != metricsMap.end()) {
                node.metrics = it->second;
            }
            
            nodes[func->name] = node;
        }
        
        // Step 2: Detect function calls and build edges
        for (auto* func : functions) {
            CallDetectorVisitor detector;
            if (func->body) {
                detector.visitBlockStatement(func->body);
            }
            
            // Add edges for all function calls
            for (const string& callee : detector.calledFunctions) {
                // Only add if callee exists in our function list
                if (nodes.find(callee) != nodes.end()) {
                    nodes[func->name].callees.push_back(callee);
                    nodes[callee].callers.push_back(func->name);
                    nodes[callee].callCount++;
                }
            }
        }
        
        // Step 3: Detect recursion
        detectRecursion();
        
        // Step 4: Calculate energies
        calculateEnergies();
    }
    
    // Get the call graph node for a function
    CallGraphNode* getNode(const string& functionName) {
        auto it = nodes.find(functionName);
        return it != nodes.end() ? &it->second : nullptr;
    }
    
    // Get all nodes sorted by total energy (highest first)
    vector<CallGraphNode*> getNodesByEnergy() {
        vector<CallGraphNode*> result;
        for (auto& pair : nodes) {
            result.push_back(&pair.second);
        }
        
        sort(result.begin(), result.end(), 
             [](CallGraphNode* a, CallGraphNode* b) {
                 return a->totalEnergy > b->totalEnergy;
             });
        
        return result;
    }
    
    // Get energy breakdown for a specific function
    map<string, double> getEnergyBreakdown(const string& functionName) {
        map<string, double> breakdown;
        
        auto* node = getNode(functionName);
        if (!node) return breakdown;
        
        // Add direct energy
        breakdown[functionName + " (direct)"] = node->directEnergy;
        
        // Add energy from each called function
        for (const string& callee : node->callees) {
            auto* calleeNode = getNode(callee);
            if (calleeNode) {
                breakdown[callee] = calleeNode->totalEnergy;
            }
        }
        
        return breakdown;
    }
    
    // Suggest optimizations
    vector<string> suggestOptimizations() {
        vector<string> suggestions;
        auto sortedNodes = getNodesByEnergy();
        
        for (auto* node : sortedNodes) {
            // Check for high-energy loops
            if (node->metrics.loopCount > 0 && node->totalEnergy > 50.0) {
                suggestions.push_back(
                    "⚠️ Function '" + node->functionName + "' has " + 
                    to_string(node->metrics.loopCount) + " loop(s) consuming " + 
                    to_string((int)node->totalEnergy) + " units. Consider loop optimization."
                );
            }
            
            // Check for frequent I/O in loops
            if (node->metrics.inputOutputCount > 0 && node->metrics.loopCount > 0) {
                suggestions.push_back(
                    "⚠️ Function '" + node->functionName + "' performs I/O inside loops. " +
                    "Consider batching I/O operations."
                );
            }
            
            // Check for recursion
            if (node->isRecursive && node->totalEnergy > 30.0) {
                suggestions.push_back(
                    "⚠️ Function '" + node->functionName + "' is recursive with high energy (" +
                    to_string((int)node->totalEnergy) + " units). Consider iterative implementation."
                );
            }
            
            // Check for frequently called functions
            if (node->callCount > 5 && node->directEnergy > 10.0) {
                suggestions.push_back(
                    "💡 Function '" + node->functionName + "' is called " + 
                    to_string(node->callCount) + " times. Consider memoization or caching."
                );
            }
        }
        
        return suggestions;
    }
    
    // Print detailed breakdown
    void printDetailedBreakdown(const string& functionName) {
        auto* node = getNode(functionName);
        if (!node) {
            cout << "Function '" << functionName << "' not found!" << endl;
            return;
        }
        
        cout << "\n╔════════════════════════════════════════════════════╗" << endl;
        cout << "║         ENERGY BREAKDOWN: " << left << setw(20) 
             << functionName << "        ║" << endl;
        cout << "╚════════════════════════════════════════════════════╝\n" << endl;
        
        // Direct energy
        cout << "Direct Energy (this function only):" << endl;
        cout << "  Operations:       " << fixed << setprecision(2) 
             << node->directEnergy << " units" << endl;
        
        // Energy from called functions
        if (!node->callees.empty()) {
            cout << "\nCalled Functions:" << endl;
            double indirectEnergy = 0.0;
            
            for (const string& callee : node->callees) {
                auto* calleeNode = getNode(callee);
                if (calleeNode) {
                    cout << "  ├─ " << left << setw(20) << callee 
                         << " " << setw(8) << right << fixed << setprecision(2)
                         << calleeNode->totalEnergy << " units" << endl;
                    indirectEnergy += calleeNode->totalEnergy;
                }
            }
            
            cout << "\nIndirect Energy (from called functions):" << endl;
            cout << "  Total:            " << fixed << setprecision(2) 
                 << indirectEnergy << " units" << endl;
        }
        
        // Total
        cout << "\n────────────────────────────────────────" << endl;
        cout << "TOTAL ENERGY:       " << fixed << setprecision(2) 
             << node->totalEnergy << " units" << endl;
        cout << "────────────────────────────────────────\n" << endl;
        
        // Called by
        if (!node->callers.empty()) {
            cout << "Called by: ";
            for (size_t i = 0; i < node->callers.size(); i++) {
                cout << node->callers[i];
                if (i < node->callers.size() - 1) cout << ", ";
            }
            cout << endl;
        }
        
        if (node->callCount > 0) {
            cout << "Call count: " << node->callCount << " times" << endl;
        }
        
        if (node->isRecursive) {
            cout << "⚠️ Recursive function (depth ~" 
                 << node->maxRecursionDepth << ")" << endl;
        }
    }
    
    // Print summary of all functions
    void printSummary() {
        auto sortedNodes = getNodesByEnergy();
        
        cout << "\n╔════════════════════════════════════════════════════╗" << endl;
        cout << "║          ENERGY SUMMARY (All Functions)           ║" << endl;
        cout << "╚════════════════════════════════════════════════════╝\n" << endl;
        
        cout << left << setw(25) << "Function" 
             << right << setw(12) << "Direct" 
             << setw(12) << "Total" 
             << setw(10) << "Calls" << endl;
        cout << "────────────────────────────────────────────────────────" << endl;
        
        for (auto* node : sortedNodes) {
            cout << left << setw(25) << node->functionName
                 << right << setw(12) << fixed << setprecision(2) << node->directEnergy
                 << setw(12) << fixed << setprecision(2) << node->totalEnergy
                 << setw(10) << node->callCount << endl;
        }
        
        cout << "────────────────────────────────────────────────────────\n" << endl;
    }
    
    // Visualize call graph (simple text format)
    void visualizeCallGraph(const string& rootFunction = "") {
        cout << "\n╔════════════════════════════════════════════════════╗" << endl;
        cout << "║              CALL GRAPH VISUALIZATION              ║" << endl;
        cout << "╚════════════════════════════════════════════════════╝\n" << endl;
        
        set<string> visited;
        
        if (!rootFunction.empty()) {
            printCallTree(rootFunction, 0, visited);
        } else {
            // Print all root functions (functions not called by anyone)
            for (auto& pair : nodes) {
                if (pair.second.callers.empty()) {
                    printCallTree(pair.first, 0, visited);
                }
            }
        }
    }

private:
    map<string, CallGraphNode> nodes;
    EnergyModel energyModel;
    
    // Detect recursive functions using DFS
    void detectRecursion() {
        for (auto& pair : nodes) {
            set<string> visited;
            if (hasRecursionDFS(pair.first, pair.first, visited, 0)) {
                pair.second.isRecursive = true;
            }
        }
    }
    
    bool hasRecursionDFS(const string& current, const string& target, 
                         set<string>& visited, int depth) {
        if (depth > 0 && current == target) {
            nodes[target].maxRecursionDepth = max(nodes[target].maxRecursionDepth, depth);
            return true;
        }
        
        if (visited.count(current)) return false;
        visited.insert(current);
        
        auto* node = getNode(current);
        if (!node) return false;
        
        for (const string& callee : node->callees) {
            if (hasRecursionDFS(callee, target, visited, depth + 1)) {
                return true;
            }
        }
        
        return false;
    }
    
    // Calculate direct and total energies
    void calculateEnergies() {
        // Calculate direct energy for each function
        for (auto& pair : nodes) {
            pair.second.directEnergy = energyModel.computeEnergy(pair.second.metrics);
        }
        
        // Calculate total energy (including called functions)
        // Use memoization to avoid recalculation
        map<string, double> memo;
        for (auto& pair : nodes) {
            pair.second.totalEnergy = calculateTotalEnergy(pair.first, memo);
        }
    }
    
    double calculateTotalEnergy(const string& functionName, map<string, double>& memo) {
        // Check memo
        if (memo.find(functionName) != memo.end()) {
            return memo[functionName];
        }
        
        auto* node = getNode(functionName);
        if (!node) return 0.0;
        
        // Start with direct energy
        double total = node->directEnergy;
        
        // Add energy from called functions
        for (const string& callee : node->callees) {
            // Avoid infinite recursion in calculation
            if (callee != functionName) {
                total += calculateTotalEnergy(callee, memo);
            }
        }
        
        memo[functionName] = total;
        return total;
    }
    
    // Print call tree recursively
    void printCallTree(const string& functionName, int depth, set<string>& visited) {
        // Print indentation
        for (int i = 0; i < depth; i++) {
            cout << "│   ";
        }
        
        auto* node = getNode(functionName);
        if (!node) return;
        
        // Print function with energy
        cout << "├── " << functionName 
             << " [" << fixed << setprecision(2) << node->totalEnergy << " units]";
        
        if (node->isRecursive) {
            cout << " (recursive)";
        }
        cout << endl;
        
        // Avoid infinite loops in visualization
        if (visited.count(functionName)) {
            for (int i = 0; i <= depth; i++) {
                cout << "│   ";
            }
            cout << "└── (already shown)" << endl;
            return;
        }
        
        visited.insert(functionName);
        
        // Print called functions
        for (const string& callee : node->callees) {
            printCallTree(callee, depth + 1, visited);
        }
    }
};

#endif