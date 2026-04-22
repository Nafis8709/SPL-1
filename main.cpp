/*#include <bits/stdc++.h>
#include "Lexer.h"
#include "Ast.h"
#include "AstVisitor.h"
#include "EnergyModel.h"
#include "Parser.h"
#include "Analyzer.h"
#include "CallGraph.h"
#include "OptimizationAdvisor.h"
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "usage: analyzer <file.c>" << endl;
        return 1;
    }

    ifstream f(argv[1]);
    if (!f) {
        cout << "error: cannot open " << argv[1] << endl;
        return 1;
    }

    string src((istreambuf_iterator<char>(f)), {});
    f.close();
    //cout << "analyzing: " << argv[1] << "\n" << endl;

    //1: tokenize
    Lexer lexer(src);
    vector<Token> tokens = lexer.tokenize();

    //2: parse functions
    Parser parser(tokens);
    vector<Function*> funcs = parser.parse();

    if (funcs.empty()) {
        cout << "no functions found." << endl;
        return 1;
    }
    cout << "found " << funcs.size() << " function(s)" << endl;

    //3: analyze each function
    map<string, Metrics> mmap;
    Analyzer analyzer;
    analyzer.setFuncs(funcs);
    for (auto* func : funcs) {
        Metrics m = analyzer.analyze(func);
        mmap[func->name] = m;
    }

    //4: build call graph + print energy results
    CallGraph cg;
    cg.build(funcs, mmap);
    cg.printSummary();
    cg.visualize();

    //5: optimization suggestions
    double total = 0.0;
    for (auto& p : mmap) total += p.second.energy;

    Advisor advisor;
    advisor.setTotal(total);
    advisor.analyze(mmap, cg);
    advisor.printSummary();
    advisor.print();

    return 0;
}*/

/*#include <bits/stdc++.h>
#include "Lexer.h"
#include "Ast.h"
#include "AstVisitor.h"
#include "EnergyModel.h"
#include "Parser.h"
#include "Analyzer.h"
#include "CallGraph.h"
#include "OptimizationAdvisor.h"
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "usage: analyzer <file.c>" << endl;
        return 1;
    }

    ifstream f(argv[1]);
    if (!f) {
        cout << "error: cannot open " << argv[1] << endl;
        return 1;
    }

    string src((istreambuf_iterator<char>(f)), {});
    f.close();

    cout << "analyzing: " << argv[1] << "\n" << endl;

    // step 1: tokenize
    Lexer lexer(src);
    vector<Token> tokens = lexer.tokenize();

    // step 2: parse functions
    Parser parser(tokens);
    vector<Function*> funcs = parser.parse();

    if (funcs.empty()) {
        cout << "no functions found." << endl;
        return 1;
    }
    cout << "found " << funcs.size() << " function(s)" << endl;

    // step 3: analyze each function
    map<string, Metrics> mmap;
    Analyzer analyzer;
    analyzer.setFuncs(funcs);
    for (auto* func : funcs) {
        Metrics m = analyzer.analyze(func);
        mmap[func->name] = m;
    }

    // step 4: build call graph + print energy results
    CallGraph cg;
    cg.build(funcs, mmap);
    cg.printSummary();
    cg.visualize();
    cg.printLoopAnalysis();

    // step 5: optimization suggestions
    double total = 0.0;
    for (auto& p : mmap) total += p.second.energy;

    Advisor advisor;
    advisor.setTotal(total);
    advisor.analyze(mmap, cg);
    advisor.printSummary();
    advisor.print();

    return 0;
}*/

#include <bits/stdc++.h>
#include "Lexer.h"
#include "Ast.h"
#include "AstVisitor.h"
#include "EnergyModel.h"
#include "Parser.h"
#include "Analyzer.h"
#include "CallGraph.h"
#include "Suggestion.h"
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "usage: analyzer <file.c>" << endl;
        return 1;
    }

    ifstream f(argv[1]);
    if (!f) {
        cout << "error: cannot open " << argv[1] << endl;
        return 1;
    }
    string src((istreambuf_iterator<char>(f)), {});
    f.close();

    cout << "analyzing: " << argv[1] << "\n" << endl;

    // step 1: tokenize
    Lexer lexer(src);
    vector<Token> tokens = lexer.tokenize();

    // step 2: parse
    Parser parser(tokens);
    vector<Function*> funcs = parser.parse();
    if (funcs.empty()) { cout << "no functions found." << endl; return 1; }
    cout << "found " << funcs.size() << " function(s)" << endl;

    // step 3: analyze each function
    map<string, Metrics> mmap;
    Analyzer analyzer;
    analyzer.setFuncs(funcs);
    for (auto* func : funcs) {
        Metrics m = analyzer.analyze(func);
        mmap[func->name] = m;
    }

    // step 4: build call graph
    CallGraph cg;
    cg.build(funcs, mmap);

    // step 5: find hotspot (highest energy, excluding main)
    GraphNode* hotspot = cg.getHotspot("main");
    string hotName = hotspot ? hotspot->name : "";

    // step 6: energy summary — all functions, hotspot marked
    if (!hotName.empty())
        cg.printSummaryWithHotspot(hotName);
    else
        cg.printSummary();

    // step 7: call graph
    cg.visualize();

    // step 8: hotspot highlight
    if (hotspot) {
        cout << "\nHOTSPOT DETECTED" << endl;
        cout << "================" << endl;
        cout << "Function  : " << hotspot->name << endl;
        cout << "Direct    : " << fixed << setprecision(2) << hotspot->directEnergy << " units" << endl;
        cout << "Total     : " << fixed << setprecision(2) << hotspot->totalEnergy  << " units" << endl;
        if (!hotspot->callees.empty()) {
            cout << "Calls     : ";
            for (size_t i = 0; i < hotspot->callees.size(); i++) {
                cout << hotspot->callees[i];
                if (i < hotspot->callees.size()-1) cout << ", ";
            }
            cout << endl;
        }
        if (hotspot->isRec)
            cout << "Note      : recursive (depth ~" << hotspot->recDepth << ")" << endl;
    }

    // step 9: detailed breakdown — hotspot only
    if (!hotName.empty())
        cg.printHotspotBreakdown(hotName);

    // step 10: optimization suggestions — hotspot only
    double total = 0.0;
    for (auto& p : mmap) total += p.second.energy;

    Advisor advisor;
    advisor.setTotal(total);
    advisor.analyze(mmap, cg);
    if (!hotName.empty())
        advisor.printHotspotSuggestions(hotName);

    return 0;
}