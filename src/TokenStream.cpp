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
            // case ' ': case '.': case '\n': case '\t': case ';': case ',': case '+':
            case ' ':case '\t':case '\n':case '\r':case '.':case '!':case '?':
            case ',':case ';':case ':':case '\'':
            case '*': case '{': case '}': case '[': case ']':case '_':case '=':
            case '#':case '$':case '%':case '^':case '~':case '`':case '/':
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
                    if (!isSkippedChar(*currChar) && currChar != endChar)
                        {
                            // This statement ensures that we don't accidentally tokenize characters we should be skipping
                            // The conditional after the || ensures that if we type a query like "the &| fox", the operators
                            // in the middle are simply ignored
                            if (isSkippedChar(*startChar) || (*startChar != op && (*startChar == '&' || *startChar == '|')) )  
                                startChar++;
                            // This statement ensures we don't accidentally skip characters we should be tokenizing
                            // e.g. query: "quick &-fox" we probably want to ignore the '&' but not the '-'
                            else if (isToken(*startChar) || isToken(*currChar))
                                continue;
                        break;
                        }
                        
                    return setCurrentToken(startChar - 1, (op == '&') ? TokenType::TokenTypeAND : TokenType::TokenTypeOR);
                    }
                // Skip most tokens at the end of a word. However, the user probably wants
                // ()" to still count as tokens, so don't skip those
                while (*currChar == '&' || *currChar == '|' || *currChar == '-')
                    {
                        ++currChar; 
                        ++count;
                    }
                // If we hit a '&' or '|' after we've already seen some other characters, we want to tokenize those as a word
                APESEARCH::string tokenWord = APESEARCH::string(startChar, 0, (currChar - count) - startChar);
                return setCurrentToken(tokenWord, TokenType::TokenTypeWord);
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
                    {
                    // Don't consider '-' by themselves, in order to count as a not token they must immediately be followed by
                    // either a phrase or paranthesis; i.e. a NOT query must be "the -quick", rather than "the - quick" or
                    if (*currChar == '&' || *currChar == '|' || isSkippedChar(*currChar) )
                        {
                        ++currChar; ++startChar;
                        continue;
                        }
                    return setCurrentToken(startChar - 1, TokenType::TokenTypeNOT);
                    }
                // Ignore all '-' found at the end of a word
                APESEARCH::string tokenWord = APESEARCH::string(startChar, 0, (currChar - count) - startChar);
                return setCurrentToken(tokenWord, TokenType::TokenTypeWord);
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
    currentTokenString = APESEARCH::string(start, 0, currChar - start);
    checkOperatorKeyword(type);
    return (currentToken = tokenFactory(currentTokenString, type)).get();
    }

// Sets the current token string and current token to tokenString
//
// Returns a pointer to currentToken's (unique_ptr) internal pointer 
Token* TokenStream::setCurrentToken(APESEARCH::string &tokenString, TokenType type)
    {
    currentTokenString = tokenString;
    checkOperatorKeyword(type);
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

// Checks the current token string for operator keywords, and if
// is matches AND, OR, or NOT, set type to the relevant token type
// relative to the operator keyword
void TokenStream::checkOperatorKeyword(TokenType &type) {
    if (currentTokenString == "AND")
        type = TokenType::TokenTypeAND;
    else if (currentTokenString == "OR")
        type = TokenType::TokenTypeOR;
    else if (currentTokenString == "NOT")
        type = TokenType::TokenTypeNOT;
}