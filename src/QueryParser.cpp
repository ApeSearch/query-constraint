#include "../include/QueryParser.h"
#include <string>
#include "assert.h"

QueryParser::QueryParser( std::string queryLine )
    {
        size_t pos = queryLine.find(' ');

        std::string requestType(queryLine, 0, pos);
        assert(requestType == "GET");

        size_t startPos = queryLine.find('=');
        size_t endPos = queryLine.find(' ', startPos);

        auto query = std::string(queryLine, startPos + 1, endPos - startPos - 1);
        stream = TokenStream(query);
    }

Token* QueryParser::FindNextToken()
    {
        auto tok = stream.TakeToken();
        if (tok->getTokenType() != TokenType::TokenTypeEOF)
            std::cout << tok->TokenString() << std::endl;
        return tok;
    }

Tuple* QueryParser::FindOrConstraint()
    {   
        TupleList* orExp = new OrExpression();
        Tuple* tuple = FindAndConstraint();

        while(FindOrOp())
            orExp->Append(FindAndConstraint()); //psuedocode passes a tokenType, why?????

        if(orExp->Top == orExp->Bottom){ //If there was no OrOp found
            return tuple;
        } 
        

        return orExp; //the memory will be deallocated after compiling into an ISR.
    }