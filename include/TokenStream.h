#include "assert.h"
#include "Token.h"
#include <unistd.h>
#include <string>

#ifndef _APESEARCH_TOKENSTREAM_H
#define _APESEARCH_TOKENSTREAM_H

    
class TokenStream {
    public:
        Token* CurrentToken(){ return currentToken; }; //case if currentToken is null
        Token* TakeToken(); //Deletes the first token in currentTokenStream
        std::string GetInput(){ return input; };
        bool Match(TokenType t);
        TokenStream() : input(), currentTokenString(), currentToken(nullptr) {}
        TokenStream(std::string _input): input(_input), currentTokenString(), currentToken(nullptr), currChar(&input.front()), endChar(input.c_str() + input.size()) {}
    
    private:
        std::string input;
        std::string currentTokenString;
        Token* currentToken;

        char const *currChar, *endChar;
};

#endif