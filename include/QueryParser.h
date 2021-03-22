#include "assert.h"
#include <unistd.h>
#include <utility>
#include <iostream>

#include "Token.h"
#include "Tuple.h"
#include "TupleList.h"
#include "TokenStream.h"


#ifndef _QUERY_PARSER_H
#define _QUERY_PARSER_H

class QueryParser
{
    public:
        QueryParser( std::string queryLine );
        
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

    private:
        TokenStream stream;
};

#endif
