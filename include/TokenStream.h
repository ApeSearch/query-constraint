#include "assert.h"
#include "Token.h"
#include <unistd.h>
#include "../libraries/AS/include/AS/string.h"

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
        
        APESEARCH::string GetInput(){ return input; };
        bool Match(TokenType t);
        bool Empty() {return endChar == currChar; }

        TokenStream() : input(), currentTokenString(), currentToken(nullptr) {}
        TokenStream(const char * _input): input(_input), currentTokenString() {
            setStreamPtrs();
        }

        TokenStream& operator=(const TokenStream& other) {
            input = other.input;
            setStreamPtrs();
            return *this;
        }

        void setStreamPtrs() {
            stream = new char[strlen(input) + 1];
            strcpy(stream, input);
            currChar = stream;
            endChar = stream + strlen(stream);
        }

        ~TokenStream() {
            delete [] stream;
        }
    
    private:
        const char *input;
        APESEARCH::string currentTokenString;

        unique_ptr<Token> currentToken;

        char *stream;
        char const *currChar, *endChar;

        Token* setCurrentToken(char const *start, TokenType type);
        Token* setCurrentToken(APESEARCH::string &tokenString, TokenType type);

        int concatenateOp(char op, char const * &start);

        void checkOperatorKeyword(TokenType &type); 

        unique_ptr<Token> tokenFactory(APESEARCH::string &token, TokenType type) 
            {
             switch(type)
                {
                case TokenType::TokenTypeAND: return unique_ptr<Token>(new TokenAND(currentTokenString));
                case TokenType::TokenTypeOR: return unique_ptr<Token>(new TokenOR(currentTokenString));
                case TokenType::TokenTypePhrase: return unique_ptr<Token>(new TokenPhrase(currentTokenString));
                case TokenType::TokenTypeWord: return unique_ptr<Token>(new TokenWord(currentTokenString));
                case TokenType::TokenTypeNOT: return unique_ptr<Token>(new TokenNOT(currentTokenString));
                case TokenType::TokenTypeNested: return unique_ptr<Token>(new TokenNested(currentTokenString));
                case TokenType::TokenTypeInvalid: return unique_ptr<Token>(new TokenInvalid());
                case TokenType::TokenTypeEOF: return unique_ptr<Token>(new TokenEOF());
                }
            }

        bool isSkippedChar (char a) {
            switch(a) {
            case ' ':
            case '.':
            case ';':
            case ',':
            case '\t':
            case '\n':
            case '\r':
            case '+':
               return true;
            default:
               return false;
            }
        }

        bool isToken (char a) {
            switch(a) {
            case '\"':
            case '-':
            case '(':
            case ')':
            case '&':
            case '|':
               return true;
            default:
               return false;
            }
        } 
};

#endif