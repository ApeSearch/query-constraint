#include "assert.h"
#include <unistd.h>
#include <utility>
#include <iostream>

#include <stdlib.h>
#include <ctype.h>

#include "Token.h"
#include "Tuple.h"
#include "TupleList.h"
#include "TokenStream.h"


#include "../libraries/AS/include/AS/algorithms.h"


#ifndef _QUERY_PARSER_H
#define _QUERY_PARSER_H

class QueryParser
{
    public:
        static APESEARCH::vector<char> decorators;
        QueryParser( APESEARCH::string queryLine );

        QueryParser(const QueryParser &other) {
            *this = other;
        }

        Token* FindNextToken(); //Gets next Token from stream

        Token* getCurrentToken() {
            return stream.getCurrentToken();
        }
        
        bool FindOrOp();
        bool FindAndOp();

        query::Tuple* FindOrConstraint();
        query::Tuple* FindAndConstraint();
        query::Tuple* FindPhrase();

        query::Tuple* FindNestedConstraint();
        query::Tuple* FindSearchWord();
        query::Tuple* FindSimpleConstraint();
        query::Tuple* FindUnarySimpleConstraint();

        static APESEARCH::string urlDecode(const char *src)
            {
            APESEARCH::string dest;
            char a, b;
            while (*src) 
                {
                if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) 
                    {
                    if (a >= 'a')
                        a -= 'a'-'A';
                    if (a >= 'A')
                        a -= ('A' - 10);
                    else
                        a -= '0';
                    if (b >= 'a')
                        b -= 'a'-'A';
                    if (b >= 'A')
                        b -= ('A' - 10);
                    else
                        b -= '0';
                    dest.push_back(16*a+b);
                    src+=3;
                    }
                else 
                    {
                    dest.push_back(*src++);
                    }
                }
                return dest;
            }

        APESEARCH::string query;

    private:
        TokenStream stream;
};

#endif
