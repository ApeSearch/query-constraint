#include <iostream>

#include "../include/TokenStream.h"

// Sets the currentToken and currentTokenString to the value of the next token in the input query
//
// Returns a pointer to the new token. NOTE THAT THIS IS RETURNS A PTR TO A UNIQUE_PTR'S INTERNAL PTR,
// AND SHOULD NOT BE MODIFIED
Token* TokenStream::TakeToken()
    {
    char const *startChar = currChar;
    while (currChar != endChar)
        {
        // Check the current character's value and whether we need to return a token
        switch(*currChar)
            {
            case ' ': case '.': case '\n': case '\t':
                {
                if (startChar != currChar)
                    // If we hit a space, '.', newline, or tab after we've already seen some other characters, we
                    // want to tokenize those as a Word
                    return setCurrentToken(startChar, TokenType::TokenTypeWord);
                else 
                    // Skip space, '.', newline, or tab if we haven't already seen any other characters
                    ++startChar;
                break;
                } // end case ' ', '.', '\n', '\t'
            case '&': case '|':
                {
                char op = *currChar;
                // Treat all concatenated '&' or '|' as a single '&' or '|' and return how many are concatenated
                int count = concatenateOp(op, startChar);
                if (startChar == currChar) 
                    {
                    // Ensures that '&' or '|' immediately followed by a character isn't tokenized as an operator
                    // However, if the current character is the end character, we do want to tokenize it as an operator
                    if (*(currChar) != ' ')
                        if (currChar != endChar)
                            break;
                        
                    return setCurrentToken(startChar - 1, (op == '&') ? TokenType::TokenTypeAND : TokenType::TokenTypeOR);
                    }
                // If we hit a '&' or '|' after we've already seen some other characters, we want to tokenize those as a word
                return setCurrentToken(std::string(startChar, (currChar - count) - startChar), TokenType::TokenTypeWord);
                } // end case '&', '|'
            case '(': case ')': case '\"':
                {
                if (startChar == currChar)
                    {
                    // Increment the current character so the next takeToken starts on the next character
                    ++currChar;
                    // Return the '\"' phrase token or the parathesis nested token, depending on what the character is 
                    return setCurrentToken(startChar, (*startChar == '\"') ? TokenType::TokenTypePhrase : TokenType::TokenTypeNested);
                    }
                // If we hit a ), (, or " after we've already seen some other characters, we want to
                // tokenize those characters as a Word
                return setCurrentToken(startChar, TokenType::TokenTypeWord);
                } // end case '(', ')', '\"'
            case '-':
                {
                // Treat all concatenated '-' as a single '-', return the number concatenated
                int count = concatenateOp('-', startChar);

                // Handle '-' at the beginning of a token
                if (startChar == currChar) 
                    return setCurrentToken(startChar - 1, TokenType::TokenTypeNOT);

                // Ignore all '-' found at the end of a word
                return setCurrentToken(std::string(startChar, (currChar - count) - startChar), TokenType::TokenTypeWord);
                } // end case '-'
            } // end switch
        ++currChar;
        } // end while

        // If startChar != currChar then there must be characters in between the two
        // that need to be tokenized
        if (startChar != currChar) 
            return setCurrentToken(startChar, TokenType::TokenTypeWord);
        else
            return setCurrentToken(startChar, TokenType::TokenTypeEOF);
    }

// Returns whether or not 't' matches the current token's type
bool TokenStream::Match(TokenType t)
    {
    return t == currentToken.get()->getTokenType();
    }

// Sets the current token string and current token to the span of characters starting
// at 'start' and ending at TokenStream::currChar.
//
// Returns a pointer to currentToken's (unique_ptr) internal pointer 
Token* TokenStream::setCurrentToken(char const *start, TokenType type)
    {
    currentTokenString = std::string(start, currChar - start);
    return (currentToken = tokenFactory(currentTokenString, type)).get();
    }

// Sets the current token string and current token to tokenString
//
// Returns a pointer to currentToken's (unique_ptr) internal pointer 
Token* TokenStream::setCurrentToken(std::string tokenString, TokenType type)
    {
    currentTokenString = tokenString;
    return (currentToken = tokenFactory(currentTokenString, type)).get();
    }

// Takes in an operator and advances currChar until currChar is no
// longer equal to the value of that operator. If start and currChar
// point to the same address, it will also advance start
//
// Returns the number of characters skipped
int TokenStream::concatenateOp(char op, char const * &start)
    {
    int count = 0;
    while (*currChar == op) 
        {
            if (start == currChar) ++start;
            ++currChar; 
            ++count; 
        }
    return count;
    }