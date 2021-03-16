#include <unistd.h>
#include <string>

#ifndef _TOKEN_H
#define _TOKEN_H

enum TokenType {
        TokenTypeInvalid = -1, TokenTypeEOF = 0,
        TokenTypeWord, TokenTypeAND, TokenTypeOR, TokenTypeNOT, TokenTypePhrase, TokenTypeNested
};

class Token {
    public:
        virtual TokenType getTokenType() const = 0;
        virtual std::string TokenString() const {
            return nullptr;
        }

        static TokenType evaluateToken(std::string input)
            {
                if (input == "%20") return TokenTypeAND;
                if (input == "%22") return TokenTypePhrase;
                if (input == "%7C") return TokenTypeOR;
                return TokenTypeInvalid;
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
            return TokenType::TokenTypeInvalid;
        }
};

class TokenWord : public Token {
    public:
        TokenWord(std::string _token) : token(_token) {}

        TokenType getTokenType() const override {
            return TokenType::TokenTypeWord;
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
            return TokenType::TokenTypeAND;
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
            return TokenType::TokenTypeOR;
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
            return TokenType::TokenTypeNOT;
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
            return TokenType::TokenTypePhrase;
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
            return TokenType::TokenTypeNested;
        };
        std::string TokenString() const override {
            return token;
        };

    private:
        std::string token;
};


#endif