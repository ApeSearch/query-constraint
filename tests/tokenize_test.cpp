#include <iostream>

#include "queries.h"
#include "../include/QueryParser.h"

int main() 
    {
        for (auto queryLine : queries) 
            {
                auto query = QueryParser(queryLine);
                Token* curr;
                while ((curr = query.FindNextToken())->getTokenType() != TokenType::TokenTypeEOF) {
                    std::cout << Token::printTokenType(curr->getTokenType()) << " " << curr->TokenString() << std::endl;
                }
                std::cout << "==========\n";
            }
    }