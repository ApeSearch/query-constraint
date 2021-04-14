#include "../include/IndexHT.h"
#include "../include/ISR.h"
#include "../include/QueryParser.h"
#include "queries.h"

#include <iostream>
using std::cout; using std::endl;

#include "../libraries/AS/include/AS/unique_ptr.h"
#include "../libraries/unit_test_framework/include/unit_test_framework/unit_test_framework.h"

APESEARCH::vector<APESEARCH::string> document1 = {
    "pig", "the", "cow", "the", "pig", "and", "all", "of", "the", "animals",
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


void printPhrase(query::TupleList* phrase) {
    query::TupleList* ptr = (query::TupleList *) phrase->Top;
    while(ptr != phrase->Bottom){
        query::SearchWord* wordPtr = (query::SearchWord *) ptr->Top;

        while(wordPtr != ptr->Bottom){
            std::cout << wordPtr->word << ' ';
            wordPtr = (query::SearchWord *) wordPtr->next;
        }

        std::cout << wordPtr->word << std::endl;
        ptr = (query::TupleList *) ptr->next;
    }

    query::SearchWord* wordPtr = (query::SearchWord *) ptr->Top;

    while(wordPtr != ptr->Bottom){
        std::cout << wordPtr->word << ' ';
        wordPtr = (query::SearchWord *) wordPtr->next;
    }

    std::cout << wordPtr->word << std::endl;
}

// TEST(ISRword_Seek) {
//     APESEARCH::unique_ptr<IndexHT> index = buildIndex();
//     APESEARCH::unique_ptr<query::Tuple> constraint = buildParseTree("the");

//     ISR* testTree = constraint->Compile(index.get());

//     ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 1);

//     ASSERT_TRUE(testTree->Seek(3));
//     ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 3);

//     ASSERT_TRUE(testTree->Seek(4));
//     ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 8);

//     ASSERT_FALSE(testTree->Seek(10));
//     ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 8);
// }

// TEST(ISRword_Next) {
//     APESEARCH::unique_ptr<IndexHT> index = buildIndex();
//     APESEARCH::unique_ptr<query::Tuple> constraint = buildParseTree("the");

//     query::TupleList *orConstraint = (query::TupleList *) constraint.get();
//     query::SearchWord* top = (query::SearchWord *) orConstraint->Top;

//     while(top != orConstraint->Bottom){
//         std::cout << top->word << ' ';
//         top = (query::SearchWord* ) top->next;
//     }

//     std::cout << top->word << std::endl;
        

//     ISR* testTree = constraint->Compile(index.get());



//     ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 1);

//     testTree->Next();
//     ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 3);

//     testTree->Next();
//     ASSERT_EQUAL(((ISRWord *) testTree)->GetCurrentPost()->loc, 8);
// }

TEST(ISRword) {
    APESEARCH::unique_ptr<IndexHT> index = buildIndex();

    APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("pig"); // doc1
    APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("and"); // doc1, doc2
    APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("do"); // none
    APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("test"); // doc2

    APESEARCH::vector<ISR *> trees = {
        constraint1->Compile(index.get()),
        constraint2->Compile(index.get()),
        constraint3->Compile(index.get()),
        constraint4->Compile(index.get())
    };

    APESEARCH::vector<APESEARCH::vector<bool>> expected = {
        {true, false},
        {true, true},
        {false, false},
        {false, true}
    };

    ISRWord *val = (ISRWord*) trees[0];

    for (int i = 0; i < trees.size(); ++i) {
        for (int j = 0; j < index->numDocs; ++j) {
            if (trees[i])   {
                ASSERT_EQUAL(expected[i][j], trees[i]->NextDocument() != nullptr); 
            }
            else {
                ASSERT_EQUAL(expected[i][j], trees[i] != nullptr);
            }
        }
        delete trees[i];
    }
}

TEST(ISRand) {
    APESEARCH::unique_ptr<IndexHT> index = buildIndex();

    APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("pig animals"); // doc1
    APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("and is"); // doc1
    APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("what the do"); // none
    APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("test is"); // none

    APESEARCH::vector<ISR *> trees = {
        constraint1->Compile(index.get()),
        constraint2->Compile(index.get()),
        constraint3->Compile(index.get()),
        constraint4->Compile(index.get())
    };

    APESEARCH::vector<APESEARCH::vector<bool>> expected = {
        {true, false},
        {false, true},
        {false, false},
        {false, true}
    };

    for (int i = 0; i < trees.size(); ++i) {
        for (int j = 0; j < index->numDocs; ++j) {
            ASSERT_EQUAL(expected[i][j], trees[i]->NextDocument() != nullptr); 
        }
        delete trees[i];
    }
}

TEST(ISRand_or) {
    APESEARCH::unique_ptr<IndexHT> index = buildIndex();
    
    APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("pig animals | test"); // doc1
    APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("donkey cow | test is"); // doc2
    APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("what the do || yooo"); // none
    // APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("what | it | do"); // none

    APESEARCH::vector<ISR *> trees = {
        constraint1->Compile(index.get()),
        constraint2->Compile(index.get()),
        constraint3->Compile(index.get()),
        // constraint4->Compile(index.get())
    };

    APESEARCH::vector<APESEARCH::vector<bool>> expected = {
        {true, true},
        {false, true},
        {false, false},
        // {false, false}
    };

    for (int i = 0; i < trees.size(); ++i) {
        for (int j = 0; j < index->numDocs; ++j) {
            ASSERT_EQUAL(expected[i][j], trees[i]->NextDocument() != nullptr); 
        }
        delete trees[i];
    }
}

TEST(ISRand_phrase) {
    APESEARCH::unique_ptr<IndexHT> index = buildIndex();

    APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("test \"the cow\""); // doc1
    APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("pig \"the cow\""); // doc1
    APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("and \"and test\""); // none
    APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("\"the pig\" \"the cow\""); // none

    APESEARCH::vector<ISR *> trees = {
        constraint1->Compile(index.get()),
        constraint2->Compile(index.get()),
        constraint3->Compile(index.get()),
        constraint4->Compile(index.get())
    };

    APESEARCH::vector<APESEARCH::vector<bool>> expected = {
        {false, false},
        {true, false},
        {false, true},
        {true, false}
    };

    for (int i = 0; i < trees.size(); ++i) {
        for (int j = 0; j < index->numDocs; ++j) {
            ASSERT_EQUAL(expected[i][j], trees[i]->NextDocument() != nullptr); 
        }
        delete trees[i];
    }
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
        for (int j = 0; j < index->numDocs; ++j) {
            ASSERT_EQUAL(expected[i][j], trees[i]->NextDocument() != nullptr); 
        }
        delete trees[i];
    }
}

TEST(ISRor_phrase) {
    APESEARCH::unique_ptr<IndexHT> index = buildIndex();

    APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("\"the pig\" | test");
    APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("\"the test\" | test");
    APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("\"the pig\" | \"the test\"");
    APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("\"hello there\" | \"the test\"");

    APESEARCH::vector<ISR *> trees = {
        constraint1->Compile(index.get()),
        constraint2->Compile(index.get()),
        constraint3->Compile(index.get()),
        constraint4->Compile(index.get())
    };

    APESEARCH::vector<APESEARCH::vector<bool>> expected = {
        {true, true},
        {false, true},
        {true, false},
        {false, false}
    };

    for (int i = 0; i < trees.size(); ++i) {
        for (int j = 0; j < index->numDocs; ++j) {
            ASSERT_EQUAL(expected[i][j], trees[i]->NextDocument() != nullptr); 
        }
        delete trees[i];
    }
}

TEST(ISRphrase) {
    APESEARCH::unique_ptr<IndexHT> index = buildIndex();

    // printIndex(index.get());

    APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("\"the\""); // 1
    APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("\"the and pig\""); // none
    APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("\"and all of the animals\""); // 1
    APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("\"the pig\""); // 1
    APESEARCH::unique_ptr<query::Tuple> constraint5 = buildParseTree("\"a and\""); // 2
    APESEARCH::unique_ptr<query::Tuple> constraint6 = buildParseTree("\"and\""); // 1, 2

    APESEARCH::vector<ISR *> trees = {
        constraint1->Compile(index.get()),
        constraint2->Compile(index.get()),
        constraint3->Compile(index.get()),
        constraint4->Compile(index.get()),
        constraint5->Compile(index.get()),
        constraint6->Compile(index.get())
    };

    APESEARCH::vector<APESEARCH::vector<bool>> expected = {
        {true, false},
        {false, false},
        {true, false},
        {true, false},
        {false, true},
        {true, true}
    };

    for (int i = 0; i < trees.size(); ++i) {
        for (int j = 0; j < index->numDocs; ++j) {
            ASSERT_EQUAL(expected[i][j], trees[i]->NextDocument() != nullptr); 
        }
        delete trees[i];
    }
}


TEST_MAIN();