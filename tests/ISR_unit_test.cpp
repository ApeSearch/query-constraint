#include "../include/IndexHT.h"
#include "../include/ISR.h"
#include "../include/QueryParser.h"
#include "queries.h"

#include <iostream>
using std::cout; using std::endl;

#include "../libraries/AS/include/AS/unique_ptr.h"
#include "../libraries/unit_test_framework/include/unit_test_framework/unit_test_framework.h"

APESEARCH::vector<IndexEntry> document1 = {
    {"pig", WordAttributeNormal}, 
    {"the", WordAttributeNormal}, 
    {"cow", WordAttributeNormal}, 
    {"the", WordAttributeNormal}, 
    {"pig", WordAttributeNormal}, 
    {"and", WordAttributeNormal}, 
    {"all", WordAttributeNormal}, 
    {"of", WordAttributeNormal}, 
    {"the", WordAttributeNormal}, 
    {"animals", WordAttributeNormal},
};

APESEARCH::vector<IndexEntry> document2 = {
    {"this", WordAttributeNormal}, 
    {"is", WordAttributeNormal}, 
    {"a", WordAttributeNormal}, 
    {"and", WordAttributeNormal}, 
    {"test", WordAttributeNormal}
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
    APESEARCH::unique_ptr<IndexHT> index(new IndexHT());

    index->addDoc("https://eecs440.com", document1, document1.size());
    index->addDoc("https://eecs441.com", document2, document2.size());

    if (print)
        printIndex(index.get());
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

APESEARCH::unique_ptr<ISR> buildISR(APESEARCH::string queryIn, IndexHT* index) {
    return APESEARCH::unique_ptr<ISR>(buildParseTree(queryIn)->Compile(index));
    
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

void checkDocuments(APESEARCH::vector<ISR *> &trees, APESEARCH::vector<APESEARCH::vector<bool>>& expected, IndexHT *index) {
    for (int i = 0; i < trees.size(); ++i) {
        APESEARCH::unique_ptr<ISREndDoc> docEnd = APESEARCH::unique_ptr<ISREndDoc>(index->getEndDocISR());
        for (int j = 0; j < index->numDocs; ++j) {
            Location start = docEnd->GetStartLocation();
            Location end = docEnd->GetEndLocation();
            ASSERT_EQUAL(expected[i][j], trees[i]->NextDocument(start, end) != nullptr); 
            docEnd->Next(end);
        }
        delete trees[i];
    }
}

TEST(ISRword) {
    APESEARCH::unique_ptr<IndexHT> index = buildIndex(true);

    APESEARCH::unique_ptr<ISR> tree = buildISR("(this is a test)", index.get());

    APESEARCH::unique_ptr<ISREndDoc> docEnd = APESEARCH::unique_ptr<ISREndDoc>(index->getEndDocISR());

    Post *post;
    Location docStartLoc = docEnd->GetStartLocation();
    Location docEndLoc   = docEnd->GetEndLocation();
    while (docStartLoc != docEndLoc) {
        docStartLoc = docEnd->GetStartLocation();
        docEndLoc =   docEnd->GetEndLocation();

        // Is the next post a match for this query?
        post = tree->NextDocument( docStartLoc, docEndLoc);
        if (post != nullptr) {
            // Seek back to beginning to begin ranking
            tree->Seek(docStartLoc, docEndLoc);
            while (post) {
                cout << post->loc << " " << tree->GetNearestWord() << endl;
                post = tree->Next(docEndLoc);
            }
        }
        
        docEnd->Next(docEndLoc);
    }
}

TEST_MAIN();