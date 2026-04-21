#ifndef LEXER_H
#define LEXER_H

#include<bits/stdc++.h>
using namespace std;

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
        while (position < length) {
            char c = peek();
            if (c == '\0') {
                break;  // EOF reached unexpectedly
            }
            
            char ch = get();
            text += ch;
            
            // Check if we've found the closing quote (and it's not escaped)
            if (!escaped && ch == quote) {
                break;
            }
            
            // Update escape flag
            if (!escaped && ch == '\\') {
                escaped = true;
            } else {
                escaped = false;
            }
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

#endif