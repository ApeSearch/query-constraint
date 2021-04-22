#include "../include/Index.h"

#include <iostream>
using std::cout; using std::endl;

#include "../libraries/AS/include/AS/unique_ptr.h"
#include "../libraries/unit_test_framework/include/unit_test_framework/unit_test_framework.h"

#include "../include/IndexFileParser.h"

#include "queries.h"

#include "../include/Builder.h"

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
    APESEARCH::vector<AnchorText> aText;
    for (int i = 0; i < 1; ++i) {
        index->addDoc("https://eecs440.com", document1, aText, document1.size());
        index->addDoc("https://eecs441.com", document2, aText, document2.size());
    }
    

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
//     Builder built = Builder("./tests/processedFiles");
// }

// TEST(build_index) {
//     const char *chunkDir = "tests/indexChunks";
//     Index search = Index(chunkDir);

//     getcwd(cwd, sizeof(cwd));

//     APESEARCH::string workingDir = cwd;

//     APESEARCH::vector<APESEARCH::string> fileNames = {
//         "/tests/indexChunks/chunk0.ape",
//         "/tests/indexChunks/chunk1.ape",
//         "/tests/indexChunks/chunk2.ape"
//     };

//     auto chunkFiles = search.getFiles();
//     for (int i = 0; i < chunkFiles.size(); ++i) {
//         APESEARCH::string expectedFile = workingDir;
//         expectedFile += fileNames[i];

//         ASSERT_EQUAL(chunkFiles[i], expectedFile);
//     }
// }

TEST(search_index) {
    const char *chunkDir = "tests/indexChunks";
    Index search = Index(chunkDir);


    APESEARCH::string queryLine = buildQuery("the");
    // APESEARCH::string queryLine1 = buildQuery("\"the pig\"");

    search.searchIndexChunks(queryLine);
    // search.searchIndexChunks(queryLine1);
    std::cout << "yeet" << std::endl;
}

// TEST(anchor_text_blob) {
//     char const *filename = "tests/apechunks/apechunk0";

//     IndexFile hashFile (filename);

//     const IndexBlob* blob = hashFile.Blob();
//     APESEARCH::string strToFind = "#list of french monarchs";

//     const SerializedAnchorText* pl = blob->FindAnchorText(strToFind);

//     std::cout << pl->Key << std::endl;
//     assert(strcmp(pl->Key, strToFind.cstr()) == 0);

//     uint8_t * ptr = (uint8_t * ) &pl->Key;
//     ptr += strlen(pl->Key) + 1;

//     while(ptr < (uint8_t * ) pl + pl->bytesRequired)
//         printf("%d ", *ptr++);
    
//     std::cout << std::endl;
// }

// TEST(build_condensed) {
//     const char *filename = "./tests/indexFiles/condensedFile0";

//     IndexFileParser parser(filename);
//     char const *filenameo = "./tests/apechunks/apefile.ape";
//     IndexFile hashFile( filenameo, parser.index );

//     const char *chunkDir = "tests/apechunks";
//     Index search = Index(chunkDir);

//     APESEARCH::string queryLine = buildQuery("the cow");
//     search.searchIndexChunks(queryLine);
// }

TEST_MAIN();