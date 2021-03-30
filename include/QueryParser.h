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

using namespace query;

class QueryParser
{
    public:
        QueryParser( std::string queryLine );

        QueryParser(const QueryParser &other) {
            *this = other;
        }

        Token* FindNextToken(); //Gets next Token from stream
        
        bool FindOrOp();
        bool FindAndOp();

        Tuple* FindOrConstraint();
        Tuple* FindAndConstraint();
        Tuple* FindPhrase();

        Tuple* FindNestedConstraint();
        Tuple* FindSearchWord();
        Tuple* FindSimpleConstraint();
        Tuple* FindUnarySimpleConstraint();

        static std::string urlDecode(const char *src)
            {
            std::string dest;
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

        std::string query;

    private:
        TokenStream stream;
};

#endif
