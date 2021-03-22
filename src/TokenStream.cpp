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
                    // Case where we have been parsing an hit a '%' sign, and should return the span from startChar to currChar as a word
                    if (startChar != currChar) {
                        if (std::string(currChar, 3) != "%20")
                            break;
                            
                        setCurrentToken(startChar);
                        return currentToken = new TokenWord(currentTokenString);
                    }
                    // Case where we started on a '%' sign, and should decode it
                    else
                        {
                        // Get the '%' and the two characters following it, e.g. %20, %22, %7C
                        int encodedTokenLength = 3;
                        std::string encodedVal = std::string(currChar, encodedTokenLength);

                        // Get the TokenType associated with the decoded ASCII token
                        TokenType type = Token::evaluateToken(encodedVal);
                        switch (type)
                            {
                            case TokenType::TokenTypeAND:
                                setCurrentToken(encodedVal);
                                return currentToken = new TokenAND(currentTokenString);

                            case TokenType::TokenTypePhrase:
                                setCurrentToken(encodedVal);
                                return currentToken = new TokenPhrase(currentTokenString);
                            
                            case TokenType::TokenTypeOR:
                                // Check for next character also being a pipe
                                if (Token::evaluateToken(std::string(currChar + encodedTokenLength, encodedTokenLength)) == TokenType::TokenTypeOR)
                                    {
                                    encodedTokenLength += encodedTokenLength;
                                    encodedVal = std::string(currChar, encodedTokenLength);
                                    }
                                if (std::string(currChar + encodedTokenLength, 3) != "%20" && (currChar + encodedTokenLength) != endChar )
                                    break;

                                setCurrentToken(encodedVal);
                                return currentToken = new TokenOR(currentTokenString);
                            default:
                                return currentToken = new TokenInvalid();
                            }
                        }
                }
            case '.':
                {
                if (startChar != currChar) 
                    {
                    setCurrentToken(startChar);
                    // If there are more '.' after the one we split on, we want to ignore those
                    return currentToken = new TokenWord(currentTokenString);
                    }
                else 
                    {
                        setCurrentToken(std::string("."));
                        // If there are more '.' after the one we split on, we want to ignore those
                        while (*currChar == '.') ++currChar;
                        return currentToken = new TokenAND(currentTokenString);
                    }
                }
            case '&': 
                {
                // 
                if (startChar == currChar) 
                    {
                    *(currChar + 1) == '&' ? (currChar += 2) : (currChar += 1);
                    if (*currChar != '%')
                        break;
                    setCurrentToken(startChar);
                    return currentToken = new TokenAND(currentTokenString);
                    }
                else 
                    {
                    break;
                    }
                }
            case '(': case ')':
                {
                if (startChar == currChar) 
                    currChar += 1;
            
                setCurrentToken(startChar);
                return currentToken = new TokenPhrase(currentTokenString);
                }
            case '-':
                {
                if (startChar == currChar) 
                    currChar += 1;
            
                setCurrentToken(startChar);
                return currentToken = new TokenNOT(currentTokenString);
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
    currChar += tokenString.size();
    currentTokenString = tokenString;
    delete currentToken;
    }