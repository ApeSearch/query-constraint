#include "../include/IndexHT.h"
#include "../include/ISR.h"
#include "../include/QueryParser.h"
#include "queries.h"

#include <iostream>
using std::cout; using std::endl;

#include "../libraries/AS/include/AS/unique_ptr.h"
#include "../libraries/unit_test_framework/include/unit_test_framework/unit_test_framework.h"

APESEARCH::vector<APESEARCH::string> document1 = {
    "the", "cow", "the", "pig", "and", "all", "of", "the", "animals",
};

APESEARCH::vector<APESEARCH::string> document2 = {
    "this", "is", "a", "test"
};


APESEARCH::unique_ptr<IndexHT> buildIndex() {
    APESEARCH::unique_ptr<IndexHT> index(new IndexHT());

    index->addDoc("https://eecs440.com", document1, document1.size(), BodyText);
    index->addDoc("https://eecs441.com", document2, document2.size(), BodyText);

    return index;
}

APESEARCH::unique_ptr<query::Tuple> buildParseTree(APESEARCH::string queryIn) {
    APESEARCH::string encodedQuery = urlencode(queryIn);
    APESEARCH::string queryBegin("GET /q=");
    APESEARCH::string queryEnd(" HTTP/1.1");

    queryBegin += encodedQuery;
    queryBegin += queryEnd;

    std::cout << queryBegin << std::endl;

    QueryParser query = QueryParser(queryBegin);

    APESEARCH::unique_ptr<query::Tuple> orConstraint(query.FindOrConstraint());

    return orConstraint;
}

TEST(ISRword_Seek) {
    APESEARCH::unique_ptr<IndexHT> index = buildIndex();
    APESEARCH::unique_ptr<query::Tuple> constraint = buildParseTree("the");

    ISR* testTree = constraint->Compile(index.get());

    ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 0);

    ASSERT_TRUE(testTree->Seek(2));
    ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 2);

    ASSERT_FALSE(testTree->Seek(3));
    ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 2);
}

TEST(ISRword_Next) {
    APESEARCH::unique_ptr<IndexHT> index = buildIndex();
    APESEARCH::unique_ptr<query::Tuple> constraint = buildParseTree("the");

    ISR* testTree = constraint->Compile(index.get());

    ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 0);

    testTree->Next();
    ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 2);

    testTree->Next();
    ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 7);
}

TEST(ISRor) {
    APESEARCH::unique_ptr<IndexHT> index = buildIndex();
    APESEARCH::unique_ptr<query::Tuple> constraint = buildParseTree("this | test");

    ISR* testTree = constraint->Compile(index.get());

    // testTree->NextDocument();
}


TEST_MAIN();