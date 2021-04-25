#include "../include/Index.h"
#include "queries.h"
#include "../include/Ranker.h"
#include "../include/Builder.h"

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
    //Builder built = Builder("./tests/condensed1/");

    const char *chunkDir = "tests/condensed/apechunk0";
    IndexFile search = IndexFile(chunkDir);

    const IndexBlob* chunk = search.Blob();

    assert(chunk->verifyIndexBlob());

    Ranker ranker(chunk, buildQuery("\"debian project\""));
    
    auto results = ranker.getTopTen();

    for(int i = 0; i < results.size(); ++i){
        std::cout << results[i].url << ' ' << results[i].rank << std::endl;
    }

    /*while (true)
        {
        Index search = Index(chunkDir);
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
        search.searchIndexChunks(queryLine.cstr());
        std::cout << "=========================================================" << std::endl;
        }*/
}
