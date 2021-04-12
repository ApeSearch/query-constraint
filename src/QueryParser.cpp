#include "../include/QueryParser.h"
#include "../libraries/AS/include/AS/string.h"
#include "assert.h"

QueryParser::QueryParser( APESEARCH::string queryLine )
    {
        size_t pos = queryLine.find(' ');
    
        APESEARCH::string requestType(queryLine, 0, pos);
        assert(requestType == "GET");

        size_t startPos = queryLine.find('=');
        size_t endPos = queryLine.find(' ', startPos);

        // Get the query starting after the equal sign up until the space before the protocol
        query = APESEARCH::string(queryLine, startPos + 1, endPos - startPos - 1);
        // Decode the encoded ASCII values from the query
        query = QueryParser::urlDecode(query.cstr());

        // Create the TokenStream
        stream = TokenStream(query.cstr());
        stream.TakeToken();

    }


Token* QueryParser::FindNextToken()
    {
        return stream.TakeToken();
    }

query::Tuple* QueryParser::FindOrConstraint()
    {   
        query::TupleList* orExp = new query::OrExpression();
        query::Tuple* andConstraint = FindAndConstraint();
        
        if(!andConstraint) {
            delete orExp;
            return nullptr;
        }
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

query::Tuple* QueryParser::FindAndConstraint()
    {
        query::TupleList* andExp = new query::AndExpression();
        query::Tuple* simpleConstraint = FindSimpleConstraint();


        while(simpleConstraint){
            andExp->Append(simpleConstraint);

            FindAndOp();
            simpleConstraint = FindSimpleConstraint();
        }

        if(!andExp->Top){
            delete andExp;
            return nullptr;
        }

        else if(andExp->Top == andExp->Bottom)
            return andExp->Top;

        return andExp; //the memory will be deallocated after compiling into an ISR.

    }

query::Tuple* QueryParser::FindPhrase()
    {
        Token* token = stream.getCurrentToken();
        
        if(token->getTokenType() != TokenTypePhrase)
            return nullptr;
        
        query::TupleList* tupleList = new query::Phrase();
        
        token = FindNextToken();
        while(token->getTokenType() != TokenTypePhrase && token->getTokenType() != TokenTypeEOF) {
            if(token->getTokenType() != TokenTypeWord)
                continue;
            query::Tuple* tuple = new query::SearchWord(token->TokenString());
            tupleList->Append(tuple);
            token = FindNextToken();
        }

        if(token->getTokenType() == TokenTypeEOF) {
            delete tupleList;
            return nullptr;
        }

        return tupleList;
    }

query::Tuple* QueryParser::FindNestedConstraint()
    {
        Token* token = stream.getCurrentToken();

        if(token->getTokenType() != TokenTypeNested)
            return nullptr;
        
        FindNextToken();
        
        query::Tuple* tuple = FindOrConstraint();

        if(!tuple)
            return nullptr;

        if(token->getTokenType() != TokenTypeNested){ //no ending parantheses
            return tuple;
        }

        FindNextToken();

        return new query::NestedConstraint(tuple);
        //return tuple;
        
    }

query::Tuple* QueryParser::FindSearchWord()
    {
        Token* token = stream.getCurrentToken();

        if(!token || token->getTokenType() != TokenTypeWord)
            return nullptr;
        
        query::SearchWord* tuple = new query::SearchWord(token->TokenString());
        FindNextToken();

        return tuple;
    }

query::Tuple* QueryParser::FindSimpleConstraint()
    {
        query::Tuple* tuple = FindUnarySimpleConstraint();

        if(!tuple)
            tuple = FindNestedConstraint();
        
        if(!tuple)
            tuple = FindPhrase();
        
        if(!tuple)
            tuple = FindSearchWord();
        
        return tuple;
    }

query::Tuple* QueryParser::FindUnarySimpleConstraint()
    {

        if(stream.getCurrentToken()->getTokenType() != TokenTypeNOT)
            return nullptr;
        
        FindNextToken();
        return new query::UnarySimpleConstraint(FindSimpleConstraint());
    }
