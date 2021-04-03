#include <iostream>

#include "queries.h"
#include "../include/QueryParser.h"

int main() 
    {
        for (auto queryLine : queries) 
        // for (auto queryLine : weirdQueries) 
            {
                auto Query = QueryParser(queryLine);
                Token* curr;
                std::cout << "Query: " << Query.query << "\n\n";
                
                while ((curr = Query.getCurrentToken())->getTokenType() != TokenTypeEOF) {
                    std::cout << Token::printTokenType(curr->getTokenType()) << " " << curr->TokenString() << std::endl;
                    Query.FindNextToken();
                }
                std::cout << Token::printTokenType(TokenTypeEOF) << std::endl;
                std::cout << "=================================================\n";
            }
    }