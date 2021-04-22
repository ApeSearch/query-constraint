#include "../include/Index.h"
#include "queries.h"

#include <string>

APESEARCH::string buildQuery(APESEARCH::string queryIn) {
    APESEARCH::string encodedQuery = urlencode(queryIn);
    APESEARCH::string queryBegin("GET /q=");
    APESEARCH::string queryEnd(" HTTP/1.1");

    queryBegin += encodedQuery;
    queryBegin += queryEnd;

    // QueryParser query = QueryParser(queryBegin);

    // APESEARCH::unique_ptr<query::Tuple> orConstraint( query.FindOrConstraint());

    return queryBegin;
}

int main() {
    const char *chunkDir = "tests/apechunks2";
    Index search = Index(chunkDir);

    while (true)
        {
        std::cout << "Enter a search query (type #quit to exit): ";
        std::string queryIn;
        std::getline (std::cin, queryIn);
        if (queryIn == "#quit")
            {
            exit(0);
            }
        else if (queryIn == "")
            {
            std::cout << "=========================================================" << std::endl;
            continue;
            }
        std::cout << std::endl;
        APESEARCH::string queryLine = buildQuery(APESEARCH::string(queryIn.cbegin(),queryIn.cend()).convertToLower());
        search.searchIndexChunks(queryLine);
        std::cout << "=========================================================" << std::endl;
        }
    
    
    

    

    
}