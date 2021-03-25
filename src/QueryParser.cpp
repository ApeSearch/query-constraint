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

        // Get the query starting after the equal sign up until the space before the protocol
        query = std::string(queryLine, startPos + 1, endPos - startPos - 1);
        // Decode the encoded ASCII values from the query
        query = QueryParser::urlDecode(query.c_str());

        // Create the TokenStream
        stream = TokenStream(query);
        stream.TakeToken();

    }

Token* QueryParser::FindNextToken()
    {
        return stream.TakeToken();
    }

Tuple* QueryParser::FindOrConstraint()
    {   
        TupleList* orExp = new OrExpression();
        Tuple* andConstraint = FindAndConstraint();
        
        if(!andConstraint)
            return nullptr;
        
        orExp->Append(andConstraint);


        while(FindOrOp() && andConstraint){
            andConstraint = FindAndConstraint();
            orExp->Append(andConstraint);
        }

        
        if(orExp->Top == orExp->Bottom)
            return orExp->Top;

        return orExp; //the memory will be deallocated after compiling into an ISR.
    }

bool QueryParser::FindAndOp()
    {   
        if(stream.getCurrentToken()->getTokenType() == TokenTypeAND){
            FindNextToken();
            return true;
        }

        return false;
    
    }

bool QueryParser::FindOrOp()
    {
        if(stream.getCurrentToken()->getTokenType() == TokenTypeOR){
            FindNextToken();
            return true;
        }
        return false;
    }

Tuple* QueryParser::FindAndConstraint()
    {
        TupleList* andExp = new AndExpression();
        Tuple* simpleConstraint = FindSimpleConstraint();


        while(simpleConstraint){
            andExp->Append(simpleConstraint);

            FindAndOp();
            simpleConstraint = FindSimpleConstraint();
        }

        if(!andExp->Top)
            return nullptr;
        
        else if(andExp->Top == andExp->Bottom)
            return andExp->Top;

        return andExp; //the memory will be deallocated after compiling into an ISR.

    }

// "the | quick.brown"

Tuple* QueryParser::FindPhrase()
    {
        Token* token = stream.getCurrentToken();
        
        if(token->getTokenType() != TokenTypePhrase)
            return nullptr;
        
        TupleList* tupleList = new Phrase();
        
        token = FindNextToken();
        while(token->getTokenType() != TokenTypePhrase && token->getTokenType() != TokenTypeEOF) {
            if(token->getTokenType() != TokenTypeWord)
                continue;
            Tuple* tuple = new SearchWord(token->TokenString());
            tupleList->Append(tuple);
            token = FindNextToken();
        }

        if(token->getTokenType() == TokenTypeEOF) {
            return nullptr;
        }

        return tupleList;
    }

Tuple* QueryParser::FindNestedConstraint()
    {
        Token* token = stream.getCurrentToken();

        if(token->getTokenType() != TokenTypeNested)
            return nullptr;
        
        FindNextToken();
        
        Tuple* tuple = FindOrConstraint();

        if(!tuple)
            return nullptr;

        if(token->getTokenType() != TokenTypeNested){ //no ending parantheses
            return tuple;
        }

        FindNextToken();

        return new NestedConstraint(tuple);
        //return tuple;
        
    }
Tuple* QueryParser::FindSearchWord()
    {
        Token* token = stream.getCurrentToken();

        if(!token || token->getTokenType() != TokenTypeWord)
            return nullptr;
        
        SearchWord* tuple = new SearchWord(token->TokenString());
        FindNextToken();

        return tuple;
    }
Tuple* QueryParser::FindSimpleConstraint()
    {
        Tuple* tuple = FindUnarySimpleConstraint();

        if(!tuple)
            tuple = FindNestedConstraint();
        
        if(!tuple)
            tuple = FindPhrase();
        
        if(!tuple)
            tuple = FindSearchWord();
        
        return tuple;
    
    }
Tuple* QueryParser::FindUnarySimpleConstraint()
    {

        if(stream.getCurrentToken()->getTokenType() != TokenTypeNOT)
            return nullptr;
        
        FindNextToken();
        return new UnarySimpleConstraint(FindSimpleConstraint());
    }
