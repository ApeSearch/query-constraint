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
        Token* FindNextToken();
        TupleList* FindOrConstraint();
        bool FindOrOp();
        TupleList* FindAndConstraint();
        Tuple* FindSimpleConstraint();
        TupleList* FindPhrase();
        Tuple* FindNestedConstraint();
        Tuple* FindSearchWord();

    private:
        TokenStream stream;
};

#endif
