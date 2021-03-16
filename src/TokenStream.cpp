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
                        currentTokenString = std::string(startChar, currChar - startChar);
                        delete currentToken;
                        currentToken = new TokenWord(currentTokenString);
                        return currentToken;
                    }
                    else
                        {
                            std::string encodedVal = std::string(currChar, 3);
                            TokenType type = Token::evaluateToken(encodedVal);
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
                currentTokenString = std::string(startChar, currChar - startChar);
            }
        else
            return nullptr;
        
    }

bool TokenStream::Match(TokenType t)
    {
    return t == currentToken->getTokenType();
    }