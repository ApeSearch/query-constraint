#include <unistd.h>



#include "../libraries/AS/include/AS/string.h"

#include "../libraries/AS/include/AS/string.h"

#ifndef _TOKEN_H
#define _TOKEN_H

enum TokenType {
        TokenTypeInvalid = -1, TokenTypeEOF = 0,
        TokenTypeWord, TokenTypeAND, TokenTypeOR, TokenTypeNOT, TokenTypePhrase, TokenTypeNested
};

class Token {
    public:
        virtual TokenType getTokenType() const = 0;
        virtual APESEARCH::string TokenString() const {
            return nullptr;
        }
        // Destructors need to be virtual esp if there are derived classes that have member variables.
        virtual ~Token() {}

        static TokenType evaluateToken(APESEARCH::string input)
            {
                if (input == "%20") return TokenTypeAND;
                if (input == "%22") return TokenTypePhrase;
                if (input == "%7C") return TokenTypeOR;
                return TokenTypeInvalid;
            }

        static APESEARCH::string printTokenType(TokenType type)
            {
            switch(type)
                {
                case TokenType::TokenTypeAND: return "TokenAND";
                case TokenType::TokenTypeOR: return "TokenOR";
                case TokenType::TokenTypePhrase: return "TokenPhrase";
                case TokenType::TokenTypeWord: return "TokenWord";
                case TokenType::TokenTypeNOT: return "TokenNOT";
                case TokenType::TokenTypeNested: return "TokenNested";
                case TokenType::TokenTypeInvalid: return "TokenInvalid";
                case TokenType::TokenTypeEOF: return "TokenEOF";
                }
            }
};

class TokenInvalid : public Token {
    public:
        TokenType getTokenType() const override{
            return TokenType::TokenTypeInvalid;
        }
};

class TokenEOF : public Token {
    public:
        TokenType getTokenType() const override{
            return TokenType::TokenTypeEOF;
        }
};

class TokenWord : public Token {
    public:
        TokenWord(APESEARCH::string &_token) : token(_token) {}

        TokenType getTokenType() const override {
            return TokenType::TokenTypeWord;
        };
        APESEARCH::string TokenString() const override {
            return token;
        };

    private:
        APESEARCH::string token;
};

class TokenAND : public Token {
    public:
        TokenAND(APESEARCH::string &_token) : token(_token) {}

        TokenType getTokenType() const override {
            return TokenType::TokenTypeAND;
        };
        APESEARCH::string TokenString() const override {
            return token;
        };

    private:
        APESEARCH::string token;
};

class TokenOR : public Token {
    public:
        TokenOR(APESEARCH::string &_token) : token(_token) {}

        TokenType getTokenType() const override {
            return TokenType::TokenTypeOR;
        };
        APESEARCH::string TokenString() const override {
            return token;
        };

    private:
        APESEARCH::string token;
};

class TokenNOT : public Token {
    public:
        TokenNOT(APESEARCH::string &_token) : token(_token) {}

        TokenType getTokenType() const override {
            return TokenType::TokenTypeNOT;
        };
        APESEARCH::string TokenString() const override {
            return token;
        };

    private:
        APESEARCH::string token;
};

class TokenPhrase : public Token {
    public:
        TokenPhrase(APESEARCH::string &_token) : token(_token) {}

        TokenType getTokenType() const override {
            return TokenType::TokenTypePhrase;
        };
        APESEARCH::string TokenString() const override {
            return token;
        };

    private:
        APESEARCH::string token;
};

class TokenNested : public Token {
    public:
        TokenNested(APESEARCH::string &_token) : token(_token) {}

        TokenType getTokenType() const override {
            return TokenType::TokenTypeNested;
        };
        APESEARCH::string TokenString() const override {
            return token;
        };

    private:
        APESEARCH::string token;
};


#endif