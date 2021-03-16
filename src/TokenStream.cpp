#include <iostream>

#include "../include/TokenStream.h"

Token* TokenStream::TakeToken()
    {
    char const *startChar = currChar;
    while (currChar != endChar)
        {
        switch(*currChar)
            {
            case '%':
                {
                    if (startChar != currChar) {
                        setCurrentToken(startChar);
                        return currentToken = new TokenWord(currentTokenString);
                    }
                    else
                        {
                        std::string encodedVal = std::string(currChar, 3);
                        TokenType type = Token::evaluateToken(encodedVal);
                        // we'll need to do better error checking for weird queries but for now...
                        switch (type)
                        {
                            case TokenType::TokenTypeAND:
                                setCurrentToken(encodedVal);
                                return currentToken = new TokenAND(currentTokenString);

                            case TokenType::TokenTypePhrase:
                                setCurrentToken(encodedVal);
                                return currentToken = new TokenPhrase(currentTokenString);
                            
                            case TokenType::TokenTypeOR:
                                setCurrentToken(encodedVal);
                                return currentToken = new TokenPhrase(currentTokenString);
                            
                            default:
                                return currentToken = new TokenInvalid();
                            }
                        }
                }
            case '.':
                {

                }
            case '&': 
                {
                    // if (*(currChar + 1) == '&') //advance if &&
                }
            case '|':
                {

                }
            case '(': case ')':
                {
                    
                }
            } // end switch
        ++currChar;
        }
        if (startChar != currChar)
            {
            setCurrentToken(startChar);
            return currentToken = new TokenWord(currentTokenString);
            }
        else
            return currentToken = new TokenEOF();
        
    }

bool TokenStream::Match(TokenType t)
    {
    return t == currentToken->getTokenType();
    }

void TokenStream::setCurrentToken(char const *start)
    {
    currentTokenString = std::string(start, currChar - start);
    delete currentToken;
    }

void TokenStream::setCurrentToken(std::string tokenString)
    {
    currChar += 3;
    currentTokenString = tokenString;
    delete currentToken;
    }