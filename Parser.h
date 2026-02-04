#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "Ast.h"

class Parser {
public:
    Parser(const vector<Token>& tokens) : tokens(tokens), position(0) {}
    vector<FunctionDefinition*> parseFunctions();
    Expression* parseExpression();

private:
    const vector<Token>& tokens;
    int position;

    Token peek(int offset = 0) const {
        int idx = position + offset;
        if (idx >= (int)tokens.size()){ 
        return tokens.back();
        }else{
        return tokens[idx];
    }

}
   Token get(){
    if(position >= (int)tokens.size()){
        return tokens.back();
    }else{
        return tokens[position++];
   }
}

  bool match(TokenType type, const string& text=""){
    Token token = peek();
    if(token.type != type){
        return false;
    }
    if(!text.empty() && token.text != text){
        return false;
    }
    if(token.type == type && (text.empty() || token.text == text)){
        get();
        return true;
    }
  }
};
#endif 
