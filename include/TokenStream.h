#include "assert.h"
#include "Token.h"
#include <unistd.h>
#include <string>

#ifndef _APESEARCH_TOKENSTREAM_H
#define _APESEARCH_TOKENSTREAM_H

    
class TokenStream {
    public:
        Token* CurrentToken(){ return currentToken; };
        Token* TakeToken();
        std::string GetInput(){ return input; };
        bool Match(TokenType t);
        TokenStream() : input(), currentTokenString(), currentToken(nullptr) {}
        TokenStream(std::string _input): input(_input), currentTokenString(), currentToken(nullptr) {}
    
    private:
        std::string input;
        std::string currentTokenString;
        Token* currentToken;
};

#endif