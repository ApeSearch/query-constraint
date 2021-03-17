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
            std::cout << Token::printTokenType(tok->getTokenType()) << " " << tok->TokenString() << std::endl;
        return tok;
    }

TupleList* QueryParser::FindOrConstraint()
    {   
        TupleList* orExp = new OrExpression();
        orExp->Append(FindAndConstraint());

        while(FindOrOp())
            orExp->Append(FindAndConstraint()); //psuedocode passes a tokenType, why?????
        

        return orExp; //the memory will be deallocated after compiling into an ISR.
    }

bool QueryParser::FindAndOp()
    {   
        bool found = false;
        while(stream.CurrentToken()->getTokenType() == TokenTypeAND && !stream.Empty()){
            if(!found)
                found = true;
            
            stream.TakeToken();
        }

        if(found)
            return true;
        return false;
    }

bool QueryParser::FindOrOp()
    {
        if(stream.CurrentToken()->getTokenType() == TokenTypeOR){
            stream.TakeToken();
            return true;
        }
        return false;
    }

TupleList* QueryParser::FindAndConstraint()
    {
        TupleList* andExp = new AndExpression();
        andExp->Append(FindSimpleConstraint());

        while(FindAndOp())
            andExp->Append(FindSimpleConstraint());
        

        return andExp; //the memory will be deallocated after compiling into an ISR.

    }

// "the | quick.brown"

TupleList* QueryParser::FindPhrase()
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
SearchWord* QueryParser::FindSearchWord()
    {
        Token* token = stream.CurrentToken();

        if(!token && !stream.Empty()){
            token = stream.TakeToken();
        }

        if(token->getTokenType() != TokenTypeWord)
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