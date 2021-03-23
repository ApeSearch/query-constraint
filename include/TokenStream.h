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
        // Token* getCurrentToken(){ return currentToken; }; //case if currentToken is null
        Token* getCurrentToken() { return currentToken.get(); }
        Token* TakeToken(); //Deletes the first token in currentTokenStream
        
        std::string GetInput(){ return input; };
        bool Match(TokenType t);
        bool Empty() {return endChar == currChar; }

        TokenStream() : input(), currentTokenString(), currentToken(nullptr) {}
        TokenStream(std::string _input): input(_input), currentTokenString() {
            setStreamPtrs();
        }

        TokenStream& operator=(const TokenStream& other) {
            input = other.input;
            setStreamPtrs();
            return *this;
        }

        void setStreamPtrs() {
            stream = new char[input.size() + 1];
            strcpy(stream, input.c_str());
            currChar = stream;
            endChar = stream + strlen(stream);
        }

        ~TokenStream() {
            delete [] stream;
        }
    
    private:
        std::string input;
        std::string currentTokenString;

        unique_ptr<Token> currentToken;

        char *stream;
        char const *currChar, *endChar;

        Token* setCurrentToken(char const *start, TokenType type);
        Token* setCurrentToken(std::string tokenString, TokenType type);

        int concatenateOp(char op, char const * &start);

        unique_ptr<Token> tokenFactory(std::string token, TokenType type) 
            {
             switch(type)
                {
                case TokenType::TokenTypeAND: return unique_ptr<Token>(new TokenAND(token));
                case TokenType::TokenTypeOR: return unique_ptr<Token>(new TokenOR(token));
                case TokenType::TokenTypePhrase: return unique_ptr<Token>(new TokenPhrase(token));
                case TokenType::TokenTypeWord: return unique_ptr<Token>(new TokenWord(token));
                case TokenType::TokenTypeNOT: return unique_ptr<Token>(new TokenNOT(token));
                case TokenType::TokenTypeNested: return unique_ptr<Token>(new TokenNested(token));
                case TokenType::TokenTypeInvalid: return unique_ptr<Token>(new TokenInvalid());
                case TokenType::TokenTypeEOF: return unique_ptr<Token>(new TokenEOF());
                }
            }
};

#endif