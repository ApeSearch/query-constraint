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
    "this", "is", "a", "and", "test"
};

void printIndex(IndexHT* index) {
    hash::HashTable<APESEARCH::string, PostingList *>::iterator itr = index->dict.begin();
    while(itr != index->dict.end()){
        if (itr->key == "%") {
            itr++; continue;
        }
        cout << itr->key << " ";
        for(size_t i = 0; i < itr->value->posts.size(); ++i) {
            cout << itr->value->posts[i]->loc << " ";
        }
        cout << endl;
        itr++;
    }

    hash::Tuple<APESEARCH::string, PostingList *> * entry = index->dict.Find("%");
    cout << endl << "% ";
    for(size_t i = 0; i < entry->value->posts.size(); ++i) {
        cout << entry->value->posts[i]->loc << " ";
    }
    cout << endl << endl;
}

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

    QueryParser query = QueryParser(queryBegin);

    APESEARCH::unique_ptr<query::Tuple> orConstraint( query.FindOrConstraint());

    return orConstraint;
}

TEST(ISRword_Seek) {
    APESEARCH::unique_ptr<IndexHT> index = buildIndex();
    APESEARCH::unique_ptr<query::Tuple> constraint = buildParseTree("the");

    ISR* testTree = constraint->Compile(index.get());

    ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 0);

    ASSERT_TRUE(testTree->Seek(2));
    ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 2);

    ASSERT_TRUE(testTree->Seek(3));
    ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 7);

    ASSERT_FALSE(testTree->Seek(10));
    ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 7);
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

    APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("this | test"); // doc2
    APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("the | this"); // doc1, doc2
    APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("what | the | do"); // doc1
    APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("what | it | do"); // none

    APESEARCH::vector<ISR *> trees = {
        constraint1->Compile(index.get()),
        constraint2->Compile(index.get()),
        constraint3->Compile(index.get()),
        constraint4->Compile(index.get())
    };

    APESEARCH::vector<APESEARCH::vector<bool>> expected = {
        {false, true},
        {true, true},
        {true, false},
        {false, false}
    };

    for (int i = 0; i < trees.size(); ++i) {
        ISR* tree = trees[i];
        for (int j = 0; j < index->numDocs; ++j) {
            ASSERT_EQUAL(expected[i][j], trees[i]->NextDocument() != nullptr); 
        }
    }
}

TEST(ISRphrase) {
    APESEARCH::unique_ptr<IndexHT> index = buildIndex();

    printIndex(index.get());

    // APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("\"hello\""); // not found
    APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("\"the and pig\""); // doc1, doc2
    // APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("what | the | do"); // doc1
    // APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("what | it | do"); // none

    APESEARCH::vector<ISR *> trees = {
        // constraint1->Compile(index.get()),
        constraint2->Compile(index.get()),
        // constraint3->Compile(index.get()),
        // constraint4->Compile(index.get())
    };

    APESEARCH::vector<APESEARCH::vector<bool>> expected = {
        // {false, false},
        {true, true},
        // {true, false},
        // {false, false}
    };

    for (int i = 0; i < trees.size(); ++i) {
        for (int j = 0; j < index->numDocs; ++j) {
            ASSERT_EQUAL(expected[i][j], trees[i]->NextDocument() != nullptr); 
        }
    }
}


TEST_MAIN();