#include <unistd.h>
#include <string>

#ifndef _TOKEN_H
#define _TOKEN_H

enum TokenType {
        TokenInvalid = -1, TokenEOF = 0,
        TokenWord, TokenAND, TokenOR, TokenNOT, TokenPhrase, TokenNested
};

class Token {
    public:
        virtual TokenType getTokenType() const;
        virtual std::string TokenString() const {
            return nullptr;
        }
};

class TokenInvalid : public Token {
    public:
        TokenType getTokenType() const override{
            return TokenType::TokenInvalid;
        }
};

class TokenEOF : public Token {
    public:
        TokenType getTokenType() const override{
            return TokenType::TokenInvalid;
        }
};

class TokenWord : public Token {
    public:
        TokenType getTokenType() const override {
            return TokenType::TokenWord;
        };
        std::string TokenString() const override {
            return token;
        };

    private:
        std::string token;
};

class TokenAND : public Token {
    public:
        TokenType getTokenType() const override {
            return TokenType::TokenAND;
        };
        std::string TokenString() const override {
            return token;
        };

    private:
        std::string token;
};

class TokenOR : public Token {
    public:
        TokenType getTokenType() const override {
            return TokenType::TokenOR;
        };
        std::string TokenString() const override {
            return token;
        };

    private:
        std::string token;
};

class TokenNOT : public Token {
    public:
        TokenType getTokenType() const override {
            return TokenType::TokenNOT;
        };
        std::string TokenString() const override {
            return token;
        };

    private:
        std::string token;
};

class TokenPhrase : public Token {
    public:
        TokenType getTokenType() const override {
            return TokenType::TokenPhrase;
        };
        std::string TokenString() const override {
            return token;
        };

    private:
        std::string token;
};

class TokenNested : public Token {
    public:
        TokenType getTokenType() const override {
            return TokenType::TokenNested;
        };
        std::string TokenString() const override {
            return token;
        };

    private:
        std::string token;
};


#endif