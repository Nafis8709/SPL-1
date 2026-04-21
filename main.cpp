#include <bits/stdc++.h>
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

    // step 5: optimization suggestions
    double total = 0.0;
    for (auto& p : mmap) total += p.second.energy;

    Advisor advisor;
    advisor.setTotal(total);
    advisor.analyze(mmap, cg);
    advisor.printSummary();
    advisor.print();

    return 0;
}