#include "../include/Index.h"

#include <iostream>
using std::cout; using std::endl;

#include "../libraries/AS/include/AS/unique_ptr.h"
#include "../libraries/unit_test_framework/include/unit_test_framework/unit_test_framework.h"

#include "queries.h"

char cwd[PATH_MAX];

APESEARCH::vector<IndexEntry> document1 = {
    {"pig", WordAttributeNormal, BodyText}, 
    {"the", WordAttributeNormal, BodyText}, 
    {"cow", WordAttributeNormal, BodyText}, 
    {"the", WordAttributeNormal, BodyText}, 
    {"pig", WordAttributeNormal, BodyText}, 
    {"and", WordAttributeNormal, BodyText}, 
    {"all", WordAttributeNormal, BodyText}, 
    {"of", WordAttributeNormal, BodyText}, 
    {"the", WordAttributeNormal, BodyText}, 
    {"animals", WordAttributeNormal, BodyText},
};

APESEARCH::vector<IndexEntry> document2 = {
    {"this", WordAttributeNormal, BodyText}, 
    {"is", WordAttributeNormal, BodyText}, 
    {"a", WordAttributeNormal, BodyText}, 
    {"and", WordAttributeNormal, BodyText}, 
    {"test", WordAttributeNormal, BodyText}
};

APESEARCH::unique_ptr<IndexHT> buildIndex(const char * filename) {
    APESEARCH::unique_ptr<IndexHT> index(new IndexHT());

    index->addDoc("https://eecs440.com", document1, document1.size());
    index->addDoc("https://eecs441.com", document2, document2.size());

    IndexFile hashFile( filename, index.get() );
    return index;
}

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

// TEST(builder) {
//     buildIndex("./tests/indexChunks/chunk0.ape");
//     buildIndex("./tests/indexChunks/chunk1.ape");
//     buildIndex("./tests/indexChunks/chunk2.ape");
// }

TEST(build_index) {
    const char *chunkDir = "tests/indexChunks";
    Index search = Index(chunkDir);

    getcwd(cwd, sizeof(cwd));

    APESEARCH::string workingDir = cwd;

    APESEARCH::vector<APESEARCH::string> fileNames = {
        "/tests/indexChunks/chunk0.ape",
        "/tests/indexChunks/chunk1.ape",
        "/tests/indexChunks/chunk2.ape"
    };

    auto chunkFiles = search.getFiles();
    for (int i = 0; i < chunkFiles.size(); ++i) {
        APESEARCH::string expectedFile = workingDir;
        expectedFile += fileNames[i];

        ASSERT_EQUAL(chunkFiles[i], expectedFile);
    }
}

TEST(search_index) {
    const char *chunkDir = "tests/indexChunks";
    Index search = Index(chunkDir);

    APESEARCH::string queryLine = buildQuery("the cow");

    search.searchIndexChunks(queryLine);
}

TEST_MAIN();