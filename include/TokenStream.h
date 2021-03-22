#include "assert.h"
#include "Token.h"
#include <unistd.h>
#include <string>

#include <cstring>

#include "../libraries/AS/include/AS/unique_ptr.h"
using APESEARCH::unique_ptr;

#ifndef _APESEARCH_TOKENSTREAM_H
#define _APESEARCH_TOKENSTREAM_H

    
class TokenStream {
    public:
        Token* getCurrentToken(){ return currentToken; }; //case if currentToken is null
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

        ~TokenStream() {
            delete [] stream;
        }
    
    private:
        void setCurrentToken(char const *start);
        void setCurrentToken(std::string tokenString);

        std::string input;
        std::string currentTokenString;

        unique_ptr<Token> currToken;
        Token* currentToken;

        char *stream;
        char const *currChar, *endChar;
};

#endif