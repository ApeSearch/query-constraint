#include <vector>
#include <iostream>

#include "../include/QueryParser.h"
#include "../libraries/AS/include/AS/string.h"
#include "queries.h"

int main () {
    std::string queryIn;

    std::cout << "Enter a query: ";
    std::getline(std::cin, queryIn);
    std::cout << std::endl;
    
    APESEARCH::string encodedQuery = urlencode(APESEARCH::string(queryIn.cbegin(), queryIn.cend()));
    std::cout << "Encoded query: " << encodedQuery << std::endl;

    // Add in the extra crap from the get request's header
    APESEARCH::string queryLine = "GET /q=";
    queryLine += encodedQuery;
    queryLine += " HTTP/1.1";

    auto Query = QueryParser(queryLine);
    Token* curr;
    std::cout << "Decoded query: " << Query.query << "\n\n";
    
    while ((curr = Query.getCurrentToken())->getTokenType() != TokenTypeEOF) {
        std::cout << Token::printTokenType(curr->getTokenType()) << " " << curr->TokenString() << std::endl;
        Query.FindNextToken();
    }
    std::cout << Token::printTokenType(TokenTypeEOF) << std::endl;
    std::cout << "=================================================\n";
}