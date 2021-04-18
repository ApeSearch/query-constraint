#include "../include/IndexHT.h"
#include "../include/ISR.h"
#include "../include/QueryParser.h"
#include "../include/Index.h"
#include "../include/IndexFileParser.h"

#include "queries.h"

#include <stdio.h>

#include <iostream>
using std::cout; using std::endl;

#include "../libraries/AS/include/AS/unique_ptr.h"
#include "../libraries/unit_test_framework/include/unit_test_framework/unit_test_framework.h"

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

APESEARCH::unique_ptr<IndexHT> buildIndex(bool print) {
    char const *filename = "./tests/indexChunk.ape";
    APESEARCH::unique_ptr<IndexHT> index(new IndexHT());

    index->addDoc("https://eecs440.com", document1, document1.size());
    index->addDoc("https://eecs441.com", document2, document2.size());

    if (print)
        printIndex(index.get());

    IndexFile hashFile( filename, index.get() );
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
            cout << wordPtr->word << ' ';
            wordPtr = (query::SearchWord *) wordPtr->next;
        }

        cout << wordPtr->word << endl;
        ptr = (query::TupleList *) ptr->next;
    }

    query::SearchWord* wordPtr = (query::SearchWord *) ptr->Top;

    while(wordPtr != ptr->Bottom){
        cout << wordPtr->word << ' ';
        wordPtr = (query::SearchWord *) wordPtr->next;
    }

    cout << wordPtr->word << endl;
}

// Puts all the word ISRs belonging to an ISR tree into a vector
// Prints each word, and the document end where this ISR was found 
void printFlattenedISR(ISR* tree, Location documentEnd) {
    APESEARCH::vector<ISR *> flattened;
    tree->Flatten(flattened);

    for (auto reader : flattened) {
        cout << reader->GetNearestWord() << " ";
    }
    cout << " " << documentEnd << endl;
}

void checkDocuments(APESEARCH::vector<ISR *> &trees, APESEARCH::vector<APESEARCH::vector<int>>& expected, const IndexBlob *index, bool verbose) {
    
    for (int i = 0; i < trees.size(); ++i) {
        APESEARCH::unique_ptr<ISREndDoc> docEnd = APESEARCH::unique_ptr<ISREndDoc>(index->getEndDocISR());
        
        cout << "Query " << i + 1 << endl;
        Post * post = trees[i]->NextDocument(docEnd.get());

        int counter = 0;
        while (post) {
            docEnd->Seek(post->loc, docEnd.get());
            cout << post->loc << endl;

            // printFlattenedISR(trees[i], docEnd->posts->curPost->loc);

            if (counter < expected[i].size()) {
                ASSERT_EQUAL(post->loc, expected[i][counter]);
            } else {
                ASSERT_EQUAL("FOUND WHEN SHOULDN'T HAVE","");
            }

            post = trees[i]->NextDocument(docEnd.get());
            ++counter;
        }

        ASSERT_EQUAL(counter, expected[i].size());

        delete trees[i];
    }
}

// TEST(ISRword) {
//     char const *filename = "./tests/indexChunk.ape";
//     IndexFile hashFile (filename);
//     const IndexBlob* blob = hashFile.Blob();

//     APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("pig"); 
//     APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("and"); 
//     APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("do"); 
//     APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("test");

//     APESEARCH::vector<ISR *> trees = {
//         constraint1->Compile(blob),
//         constraint2->Compile(blob),
//         constraint3->Compile(blob),
//         constraint4->Compile(blob)
//     };

//     APESEARCH::vector<APESEARCH::vector<int>> expected = {
//         {0},
//         {5, 14},
//         {},
//         {15}
//     };

//     checkDocuments(trees, expected, blob, false);
// }

// TEST(ISRand) {
//     char const *filename = "./tests/indexChunk.ape";
//     IndexFile hashFile (filename);
//     const IndexBlob* blob = hashFile.Blob();

//     APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("pig animals"); 
//     APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("and is"); 
//     APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("what the do");
//     APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("test is");
//     APESEARCH::unique_ptr<query::Tuple> constraint5 = buildParseTree("the cow");
    

//     APESEARCH::vector<ISR *> trees = {
//         constraint1->Compile(blob),
//         constraint2->Compile(blob),
//         constraint3->Compile(blob),
//         constraint4->Compile(blob),
//         constraint5->Compile(blob)
//     };

//     APESEARCH::vector<APESEARCH::vector<int>> expected = {
//         {0},
//         {12},
//         {},
//         {12},
//         {1}
//     };

//     checkDocuments(trees, expected, blob, false);
// }

// TEST(ISRand_or) {
//     char const *filename = "./tests/indexChunk.ape";
//     IndexFile hashFile (filename);
//     const IndexBlob* blob = hashFile.Blob();
    
//     APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("pig animals | test"); // doc1
//     APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("donkey cow | test is"); // doc2
//     APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("what the do || yooo"); // none
//     APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("what | it | do"); // none

//     APESEARCH::vector<ISR *> trees = {
//         constraint1->Compile(blob),
//         constraint2->Compile(blob),
//         constraint3->Compile(blob),
//         constraint4->Compile(blob)
//     };

//     APESEARCH::vector<APESEARCH::vector<int>> expected = {
//         {0, 15},
//         {12},
//         {},
//         {}
//     };

//     checkDocuments(trees, expected, blob, false);
// }

// TEST(ISRand_phrase) {
//     char const *filename = "./tests/indexChunk.ape";
//     IndexFile hashFile (filename);
//     const IndexBlob* blob = hashFile.Blob();

//     APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("test \"the cow\""); // none
//     APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("pig \"the cow\""); // doc1
//     APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("and \"and test\""); // none
//     APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("\"the pig\" \"the cow\""); // none

//     APESEARCH::vector<ISR *> trees = {
//         constraint1->Compile(blob),
//         constraint2->Compile(blob),
//         constraint3->Compile(blob),
//         constraint4->Compile(blob)
//     };

//     APESEARCH::vector<APESEARCH::vector<int>> expected = {
//         {},
//         {0},
//         {14},
//         {1}
//     };

//     checkDocuments(trees, expected, blob, false);
// }

// TEST(ISRor) {
//     char const *filename = "./tests/indexChunk.ape";
//     IndexFile hashFile (filename);
//     const IndexBlob* blob = hashFile.Blob();

//     APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("this | test"); // doc2
//     APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("the | this"); // doc1, doc2
//     APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("what | the | do"); // doc1
//     APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("what | it | do"); // none

//     APESEARCH::vector<ISR *> trees = {
//         constraint1->Compile(blob),
//         constraint2->Compile(blob),
//         constraint3->Compile(blob),
//         constraint4->Compile(blob)
//     };

//     APESEARCH::vector<APESEARCH::vector<int>> expected = {
//         {11},
//         {1, 11},
//         {1},
//         {}
//     };

//     checkDocuments(trees, expected, blob, false);
// }

// TEST(ISRor_phrase) {
//     char const *filename = "./tests/indexChunk.ape";
//     IndexFile hashFile (filename);
//     const IndexBlob* blob = hashFile.Blob();

//     APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("\"the pig\" | test");
//     APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("\"the test\" | test");
//     APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("\"the pig\" | \"the test\"");
//     APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("\"hello there\" | \"the test\"");

//     APESEARCH::vector<ISR *> trees = {
//         constraint1->Compile(blob),
//         constraint2->Compile(blob),
//         constraint3->Compile(blob),
//         constraint4->Compile(blob)
//     };

//     APESEARCH::vector<APESEARCH::vector<int>> expected = {
//         {3, 15},
//         {15},
//         {3},
//         {}
//     };

//     checkDocuments(trees, expected, blob, false);
// }

// TEST(ISRphrase) {
//     char const *filename = "./tests/indexChunk.ape";
//     IndexFile hashFile (filename);
//     const IndexBlob* blob = hashFile.Blob();

//     APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("\"the\""); 
//     APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("\"the and pig\""); 
//     APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("\"and all of the animals\""); 
//     APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("\"the pig\""); 
//     APESEARCH::unique_ptr<query::Tuple> constraint5 = buildParseTree("\"a and\""); 
//     APESEARCH::unique_ptr<query::Tuple> constraint6 = buildParseTree("\"and\""); 

//     APESEARCH::vector<ISR *> trees = {
//         constraint1->Compile(blob),
//         constraint2->Compile(blob),
//         constraint3->Compile(blob),
//         constraint4->Compile(blob),
//         constraint5->Compile(blob),
//         constraint6->Compile(blob)
//     };

//     APESEARCH::vector<APESEARCH::vector<int>> expected = {
//         {1},
//         {},
//         {5},
//         {3},
//         {13},
//         {5, 14}
//     };

//     checkDocuments(trees, expected, blob, false);
// }

TEST(ISRphrase_2) {
    char const *filename = "./tests/testIndexBlobLarge.txt";
    IndexFile hashFile (filename);
    const IndexBlob* blob = hashFile.Blob();

    APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("\"york city\""); 
    // APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("\"the and pig\""); 
    // APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("\"and all of the animals\""); 
    // APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("\"the pig\""); 
    // APESEARCH::unique_ptr<query::Tuple> constraint5 = buildParseTree("\"a and\""); 
    // APESEARCH::unique_ptr<query::Tuple> constraint6 = buildParseTree("\"and\""); 

    APESEARCH::vector<ISR *> trees = {
        constraint1->Compile(blob),
        // constraint2->Compile(blob),
        // constraint3->Compile(blob),
        // constraint4->Compile(blob),
        // constraint5->Compile(blob),
        // constraint6->Compile(blob)
    };

    APESEARCH::vector<APESEARCH::vector<int>> expected = {
        {1},
        // {},
        // {5},
        // {3},
        // {13},
        // {5, 14}
    };

    checkDocuments(trees, expected, blob, false);
}

// literally just the same test as ISRAnd
// TEST(ISRcontained) {
//     APESEARCH::unique_ptr<IndexHT> index = buildIndex(false);

//     APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("(pig animals)"); // doc1
//     APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("(and is)"); // doc1
//     APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("(what the do)"); // none
//     APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("(test is)"); // none

//     APESEARCH::vector<ISR *> trees = {
//         constraint1->Compile(blob),
//         constraint2->Compile(blob),
//         constraint3->Compile(blob),
//         constraint4->Compile(blob)
//     };

//     APESEARCH::vector<APESEARCH::vector<bool>> expected = {
//         {true, false},
//         {false, true},
//         {false, false},
//         {false, true}
//     };

//     checkDocuments(trees, expected, blob, false);
// }

// TEST(ISRexcluded) {
//     APESEARCH::unique_ptr<IndexHT> index = buildIndex(false);

//     APESEARCH::unique_ptr<query::Tuple> constraint1 = buildParseTree("fox -the"); // doc1
//     APESEARCH::unique_ptr<query::Tuple> constraint2 = buildParseTree("and -(fox)"); // doc1
//     APESEARCH::unique_ptr<query::Tuple> constraint3 = buildParseTree("and -(fox test)"); // none
//     APESEARCH::unique_ptr<query::Tuple> constraint4 = buildParseTree("(test is)"); // none

//     APESEARCH::vector<ISR *> trees = {
//         constraint1->Compile(blob),
//         constraint2->Compile(blob),
//         constraint3->Compile(blob),
//         constraint4->Compile(blob)
//     };

//     APESEARCH::vector<APESEARCH::vector<bool>> expected = {
//         {false, false},
//         {false, true},
//         {false, false},
//         {false, true}
//     };

//     checkDocuments(trees, expected, blob, false);
// }

TEST_MAIN();