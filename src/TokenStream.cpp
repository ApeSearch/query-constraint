#include <iostream>

#include "../include/TokenStream.h"

Token* TokenStream::TakeToken()
    {

    char const *startChar = currChar;
    while (currChar != endChar)
        {
        switch(*currChar)
            {
           
            case ' ': case '.':
                if (startChar != currChar)
                    return setCurrentToken(startChar, TokenType::TokenTypeWord);
                else 
                    ++startChar;
                break;
            case '&': case '|':
                {
                // 
                if (startChar == currChar) 
                    {
                    char op = *currChar;
                    *(currChar + 1) == op ? (currChar += 2) : (currChar += 1);
                    if (*(currChar) != ' ')
                        break;
                    return setCurrentToken(startChar, (op == '&') ? TokenType::TokenTypeAND : TokenType::TokenTypeOR);
                    }
                else {
                    if (*currChar == '|' || *currChar == '&')
                        break;
                    return setCurrentToken(startChar, TokenType::TokenTypeWord);
                }
                }
            case '(': case ')': case '\"':
                {
                if (startChar == currChar)
                    {
                    currChar += 1;
                    return setCurrentToken(startChar, (*startChar == '\"') ? TokenType::TokenTypePhrase : TokenType::TokenTypeNested);
                    }
                else
                    return setCurrentToken(startChar, TokenType::TokenTypeWord);
                }
            case '-':
                {
                    if (startChar == currChar) 
                        {
                        currChar += 1;
                        return setCurrentToken(startChar, TokenType::TokenTypeNOT);
                        }
                    else 
                        {
                        ++currChar;
                        // handles the case 
                        if (*currChar != ' ')
                            return setCurrentToken(std::string(startChar, (currChar - 1) - startChar), TokenType::TokenTypeWord);
                        break;
                        }
                }
            } // end switch
            if (currChar != endChar)
                ++currChar;
            }

            if (startChar != currChar) {
                std::string endString = std::string(startChar, currChar - startChar);
                if (endString == "|" || endString == "||") 
                    return setCurrentToken(endString, TokenType::TokenTypeOR);
                else if (endString == "&" || endString == "&&") 
                    return setCurrentToken(endString, TokenType::TokenTypeAND);
            return setCurrentToken(startChar, TokenType::TokenTypeWord);
        }
        else
            return setCurrentToken(startChar, TokenType::TokenTypeEOF);
    }

bool TokenStream::Match(TokenType t)
    {
    return t == currentToken.get()->getTokenType();
    }

Token* TokenStream::setCurrentToken(char const *start, TokenType type)
    {
    currentTokenString = std::string(start, currChar - start);
    return (currentToken = tokenFactory(currentTokenString, type)).get();
    }

Token* TokenStream::setCurrentToken(std::string tokenString, TokenType type)
    {
    currentTokenString = tokenString;
    return (currentToken = tokenFactory(currentTokenString, type)).get();
    }