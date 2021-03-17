#include "assert.h"
#include "Token.h"
#include <unistd.h>
#include <string>

#include <cstring>

#ifndef _APESEARCH_TOKENSTREAM_H
#define _APESEARCH_TOKENSTREAM_H

    
class TokenStream {
    public:
        Token* CurrentToken(){ return currentToken; }; //case if currentToken is null
        Token* TakeToken(); //Deletes the first token in currentTokenStream
        std::string GetInput(){ return input; };
        bool Match(TokenType t);
        bool Empty() {return endChar == currChar; }

        TokenStream() : input(), currentTokenString(), currentToken(nullptr) {}
        TokenStream(std::string _input): input(_input), currentTokenString(), currentToken(nullptr), stream(new char[input.size() + 1]) {
            strcpy(stream, input.c_str());
            currChar = stream;
            endChar = stream + strlen(stream);
        }
    
    private:
        void setCurrentToken(char const *start);
        void setCurrentToken(std::string tokenString);

        std::string input;
        std::string currentTokenString;
        Token* currentToken;

        char *stream;
        char const *currChar, *endChar;
};

#endif