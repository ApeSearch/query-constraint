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
    }

Token* QueryParser::FindNextToken()
    {
        return stream.TakeToken();
    }

Tuple* QueryParser::FindOrConstraint()
    {   
        TupleList* orExp = new OrExpression();
        orExp->Append(FindAndConstraint());

        while(FindOrOp())
            orExp->Append(FindAndConstraint()); //psuedocode passes a tokenType, why?????


        if(!orExp->Top)
            return nullptr;
        
        else if(orExp->Top == orExp->Bottom)
            return orExp->Top;

        return orExp; //the memory will be deallocated after compiling into an ISR.
    }

bool QueryParser::FindAndOp()
    {   
        bool found = false;
        if (stream.TakeToken()->getTokenType() == TokenTypeAND && !stream.Empty()){
            if(!found)
                found = true;
            
            // stream.TakeToken();
        }
    
        if(found)
            return true;
        return false;
    }

bool QueryParser::FindOrOp()
    {
        if(stream.getCurrentToken()->getTokenType() == TokenTypeOR){
            stream.TakeToken();
            return true;
        }
        return false;
    }

Tuple* QueryParser::FindAndConstraint()
    {
        TupleList* andExp = new AndExpression();
        andExp->Append(FindSimpleConstraint());

        while(FindAndOp())
            andExp->Append(FindSimpleConstraint());

        if(!andExp->Top)
            return nullptr;
        
        else if(andExp->Top == andExp->Bottom)
            return andExp->Top;

        return andExp; //the memory will be deallocated after compiling into an ISR.

    }

// "the | quick.brown"

Tuple* QueryParser::FindPhrase()
    {
        TupleList* tupleList = new Phrase();
        Token* token = FindNextToken();
        
        if(token->getTokenType() != TokenTypePhrase)
            return nullptr;
        
        token = FindNextToken();
        while(token->getTokenType() == TokenTypeWord) {
            Tuple* tuple = new SearchWord(token->TokenString());
            tupleList->Append(tuple);
        }

        return tupleList;
    }

Tuple* QueryParser::FindNestedConstraint()
    {
        Token* token = FindNextToken();

        if(token->getTokenType() != TokenTypeNested)
            return nullptr;
        
        Tuple* tuple = FindOrConstraint();

        if(!tuple)
            return nullptr;
        
        token = FindNextToken();

        if(token->getTokenType() != TokenTypeNested){ //no ending parantheses
            return tuple;
        }

        //return new NestedConstraint(tuple);
        return tuple;
        
    }
Tuple* QueryParser::FindSearchWord()
    {
        Token* token = stream.getCurrentToken();

        if(!token && !stream.Empty()){
            token = stream.TakeToken();
        }

        if(!token || token->getTokenType() != TokenTypeWord)
            return nullptr;
        
        
        SearchWord* tuple = new SearchWord(token->TokenString());
        stream.TakeToken();

        return tuple;
    }
Tuple* QueryParser::FindSimpleConstraint()
    {
        Tuple* tuple = FindSearchWord();

        if(!tuple)
            tuple = FindUnarySimpleConstraint();
        
        if(!tuple)
            tuple = FindPhrase();
        
        if(!tuple)
            tuple = FindNestedConstraint();
        
        return tuple;
    
    }
Tuple* QueryParser::FindUnarySimpleConstraint()
    {
        Token* token = FindNextToken();

        if(token->getTokenType() != TokenTypeNOT)
            return nullptr;
        
        return FindSimpleConstraint();
    }