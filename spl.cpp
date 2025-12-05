#include<bits/stdc++.h>
using namespace std;

struct FunctionMetrics{
    int loc=0;
    int comparisonCount=0;
    int arithmeticCount=0;
    int logicalCount=0;
    int inputOutputCount=0;
    int bitwiseCount=0;
    int memoryAccessCount=0;
    int allocationCount=0;
    int loopCount=0;
    int recursionFlag=0;
    int functionCallCount=0;
    long long estimatediterationCount=0;
    double energyConsumption=0.0;
};

struct EnergyModel{
    double energyPerComparison=0.5;
    double energyPerArithmetic=0.3;
    double energyPerLogical=0.2;
    double energyPerInputOutput=1.0;
    double energyPerBitwise=0.4;
    double energyPerMemoryAccess=0.6;
    double energyPerAllocation=2.0;
    //double energyPerLoopIteration=0.1;
    double energyPerFunctionCall=0.8;
    double energyPerlineOfCode=0.05;

    double computeEnergy(const FunctionMetrics& metrics){
        if(metrics.loc == 0) return 0.0;
        if(metrics.estimatediterationCount>0){
            double LoopEnergy = (metrics.arithmeticCount + metrics.logicalCount +
                                 metrics.bitwiseCount + metrics.memoryAccessCount) *
                                 energyPerArithmetic * metrics.estimatediterationCount;
            return LoopEnergy +
                   (metrics.comparisonCount * energyPerComparison) +
                     (metrics.inputOutputCount * energyPerInputOutput) +
                        (metrics.allocationCount * energyPerAllocation) +
                           (metrics.loc * energyPerlineOfCode) +
                              (metrics.functionCallCount * energyPerFunctionCall);

        }
        if (metrics.recursionFlag){
            double RecursionEnergy = (metrics.arithmeticCount + metrics.logicalCount +
                                      metrics.bitwiseCount + metrics.memoryAccessCount) *
                                      energyPerArithmetic * 10;//recursion depth= 10
            return RecursionEnergy +
                   (metrics.comparisonCount * energyPerComparison) +
                     (metrics.inputOutputCount * energyPerInputOutput) +
                        (metrics.allocationCount * energyPerAllocation) +
                           (metrics.loc * energyPerlineOfCode) +
                              (metrics.functionCallCount * energyPerFunctionCall);
        }
        return (metrics.comparisonCount * energyPerComparison) +
               (metrics.arithmeticCount * energyPerArithmetic) +
                 (metrics.logicalCount * energyPerLogical) +
                    (metrics.inputOutputCount * energyPerInputOutput) +
                       (metrics.bitwiseCount * energyPerBitwise) +
                          (metrics.memoryAccessCount * energyPerMemoryAccess) +
                             (metrics.allocationCount * energyPerAllocation) +
                                (metrics.loc * energyPerlineOfCode) +
                                   (metrics.functionCallCount * energyPerFunctionCall);

    };

enum class TokenType {
    IDENTIFIER, KEYWORD, NUMBER, STRING, CHAR,
    OPERATOR, PUNCTUATION, END_OF_FILE, UNKNOWN
};

struct Token {
    TokenType type = TokenType::UNKNOWN;
    string text;
    int lineNumber = 0;
    int columnNumber = 0;
};

class Lexer {
public:
    Lexer(const string& src) : sourceCode(src), length(sourceCode.size()), position(0), lineNumber(1), columnNumber(1) {}
    
    vector<Token> tokenize();
    Token getNextToken();

private:
    const string sourceCode;
    int length;
    int position;
    int lineNumber;
    int columnNumber;
    bool isKeyword(const string& word) const {
        static const unordered_set<string> keywords = {
            "if", "else", "for", "while", "do", "switch", "case", "default", "break", "continue",
            "return", "int", "long", "short", "char", "float", "double", "void", "const", "static",
            "struct", "union", "enum", "typedef", "sizeof", "volatile", "signed", "unsigned", "auto",
            "extern", "register", "goto", "inline"
        };
        return keywords.count(word) > 0;
    }
    bool isPunctuation(char c) const {
        static const unordered_set<char> punctuations = {
            '(', ')', '{', '}', '[', ']', ';', ',', '#'
        };
        return punctuations.count(c) > 0;
    }
    const vector<string>& getOperators() const {
        static const vector<string> ops = {
            ">>>", ">>=", "<<=", "->", "==", "!=", "<=", ">=", "&&", "||",
            "++", "--", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<", ">>",
            "+", "-", "*", "/", "%", "<", ">", "=", "!", "~", "&", "|", "^", "?", ":", "."
        };
        return ops;
    }

    char peek(int offset = 0) const {
        int idx = position + offset;
        return idx < length ? sourceCode[idx] : '\0';
    }

    char get() {
        if (position >= length) return '\0';
        char currentChar = sourceCode[position++];
        if (currentChar == '\n') {
            lineNumber++;
            columnNumber = 1;
        } else {
            columnNumber++;
        }
        return currentChar;
    }

    void advance(int steps) {
        while (steps-- > 0) get();
    }

    void skipWhitespaceAndComments() {
        while (true) {
            char c = peek();
            if (c == '\0') return;
            
            if (isspace(c)) {
                get();
                continue;
            }
            
            if (c == '/' && peek(1) == '/') {
                get(); get();
                while (peek() != '\0' && peek() != '\n') get();
                continue;
            }
            
            if (c == '/' && peek(1) == '*') {
                get(); get();
                while (true) {
                    if (peek() == '\0') return;
                    if (peek() == '*' && peek(1) == '/') {
                        get(); get();
                        break;
                    }
                    get();
                }
                continue;
            }
            
            break;
        }
    }

    Token makeToken(TokenType type, const string& text, int line, int col) {
        Token token;
        token.type = type;
        token.text = text;
        token.lineNumber = line;
        token.columnNumber = col;
        return token;
    }

    Token readIdentifierOrKeyword() {
        int startLine = lineNumber;
        int startCol = columnNumber;
        string text;
        
        while (true) {
            char c = peek();
            if (isalpha(c) || c == '_' || isdigit(c)) {
                text += get();
            } else {
                break;
            }
        }
        
        if (isKeyword(text)) {
            return makeToken(TokenType::KEYWORD, text, startLine, startCol);
        }
        return makeToken(TokenType::IDENTIFIER, text, startLine, startCol);
    }

    Token readNumber() {
        int startLine = lineNumber;
        int startCol = columnNumber;
        string text;
        bool seenDot = false;
        bool seenExp = false;
        
        if (peek() == '0' && (peek(1) == 'x' || peek(1) == 'X')) {
            text += get(); text += get();
            while (isxdigit(peek())) text += get();
            return makeToken(TokenType::NUMBER, text, startLine, startCol);
        }
        
        while (true) {
            char c = peek();
            if (isdigit(c)) {
                text += get();
                continue;
            }
            if (!seenDot && c == '.') {
                seenDot = true;
                text += get();
                continue;
            }
            if (!seenExp && (c == 'e' || c == 'E')) {
                seenExp = true;
                text += get();
                if (peek() == '+' || peek() == '-') text += get();
                continue;
            }
            break;
        }
        
        return makeToken(TokenType::NUMBER, text, startLine, startCol);
    }

    Token readStringOrChar() {
        int startLine = lineNumber;
        int startCol = columnNumber;
        char quote = get();
        string text;
        text += quote;
        
        bool escaped = false;
        while (true) {
            char c = peek();
            if (c == '\0') {
                text += get();
                break;
            }
            char ch = get();
            text += ch;
            if (!escaped && ch == quote) break;
            if (!escaped && ch == '\\') escaped = true;
            else escaped = false;
        }
        
        TokenType kind = (quote == '"') ? TokenType::STRING : TokenType::CHAR;
        return makeToken(kind, text, startLine, startCol);
    }

    Token readOperatorOrPunctuation() {
        int startLine = lineNumber;
        int startCol = columnNumber;
        
        for (int len = 3; len >= 1; --len) {
            string candidate;
            for (int j = 0; j < len; ++j) {
                char c = peek(j);
                if (c == '\0') break;
                candidate += c;
            }
            
            if ((int)candidate.length() == len) {
                for (const string& op : getOperators()) {
                    if (op == candidate) {
                        string text;
                        for (int k = 0; k < len; ++k) text += get();
                        return makeToken(TokenType::OPERATOR, text, startLine, startCol);
                    }
                }
            }
        }
        
        char c = peek();
        if (isPunctuation(c)) {
            string text;
            text += get();
            return makeToken(TokenType::PUNCTUATION, text, startLine, startCol);
        }
        
        string text;
        text += get();
        return makeToken(TokenType::UNKNOWN, text, startLine, startCol);
    }

public:
    vector<Token> tokenize() {
        vector<Token> tokens;
        
        while (true) {
            skipWhitespaceAndComments();
            char c = peek();
            
            if (c == '\0') {
                tokens.push_back(makeToken(TokenType::END_OF_FILE, "", lineNumber, columnNumber));
                break;
            }
            
            if (isalpha(c) || c == '_') {
                tokens.push_back(readIdentifierOrKeyword());
                continue;
            }
            
            if (isdigit(c)) {
                tokens.push_back(readNumber());
                continue;
            }
            
            if (c == '"' || c == '\'') {
                tokens.push_back(readStringOrChar());
                continue;
            }
            
            tokens.push_back(readOperatorOrPunctuation());
        }
        
        return tokens;
    }

    Token getNextToken() {
        skipWhitespaceAndComments();
        char c = peek();
        
        if (c == '\0') return makeToken(TokenType::END_OF_FILE, "", lineNumber, columnNumber);
        if (isalpha(c) || c == '_') return readIdentifierOrKeyword();
        if (isdigit(c)) return readNumber();
        if (c == '"' || c == '\'') return readStringOrChar();
        
        
        return readOperatorOrPunctuation();
    }
};

};