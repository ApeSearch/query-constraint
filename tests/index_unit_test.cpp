#include "../include/IndexHT.h"
#include "../include/Index.h"
#include "../include/IndexFileParser.h"
#include <iostream>

#include "../libraries/unit_test_framework/include/unit_test_framework/unit_test_framework.h"


TEST(postingList_append)
    {
    size_t NUM_POSTS = 10;
    WordPostingList posts;
    for (Location loc = 0; loc < NUM_POSTS; ++loc)
        posts.appendToList(loc, WordAttributeNormal, 0);
    
    ASSERT_EQUAL(posts.posts.size(), NUM_POSTS);
    }

TEST(postingList_seek)
    {
    WordPostingList posts;
    for (Location loc = 0; loc < 10; ++loc)
        posts.appendToList(loc, WordAttributeNormal, 0);

    APESEARCH::vector<Location> locations = {
        0, 5, 8
    }; 

    for (int i = 0; i < locations.size(); ++i)
        {
        Post *found = posts.Seek(locations[i]);
        ASSERT_EQUAL(found->loc, locations[i]);
        }

    Post *notFound = posts.Seek(10);
    ASSERT_EQUAL(notFound, nullptr);
    }

/*
TEST(build_with_file){
    const char *filename = "./tests/indexFiles/indexFile1.txt";

    IndexFileParser parser(filename);


    APESEARCH::string s1 = "continue";
    APESEARCH::string s2 = "%the";
    APESEARCH::string s3 = "newsletter";
    APESEARCH::string s4 = "the";

    hash::Tuple<APESEARCH::string, PostingList *> * entry1 = parser.index->dict.Find(s1);
    hash::Tuple<APESEARCH::string, PostingList *> * entry2 = parser.index->dict.Find(s2);
    hash::Tuple<APESEARCH::string, PostingList *> * entry3 = parser.index->dict.Find(s3);
    hash::Tuple<APESEARCH::string, PostingList *> * entry4 = parser.index->dict.Find(s4);
    hash::HashTable<APESEARCH::string, PostingList *>::iterator itr = parser.index->dict.begin();

    assert(entry1->value->posts[0]->loc == 12);
    assert(entry2->value->posts[0]->loc == 0);
    
    assert(entry3->value->posts[0]->loc == 689);
    assert(entry3->value->posts[1]->loc == 716);

    /*
    while(itr != parser.index->dict.end()){
        std::cout << itr->key << std::endl;
        for(size_t i = 0; i < itr->value->posts.size(); ++i) {
            std::cout << itr->value->posts[i]->loc << "-" << itr->value->posts[i]->tData << ' ';
        }
        std::cout << std::endl;
        itr++;
    }
    
    entry4->value->posts.push_back(new WordPost(1000000, WordAttributeNormal));
    size_t bytesRequired = parser.index->BytesRequired();

    size_t bytes = entry4->value->bytesRequired("the");

    WordPostingList * theList = (WordPostingList * )entry4->value;

    char const *filenameo = "./tests/testIndexBlobLarge.txt";
    IndexFile hashFile( filenameo, parser.index );

    std::cout << "PASS" << std::endl;

}*/


TEST(basic_encode_deltas_bytes){
     APESEARCH::vector<IndexEntry> words = {
        {"the", WordAttributeNormal, BodyText},
        {"cow", WordAttributeNormal, BodyText},
        {"the", WordAttributeBold, BodyText},
        {"pig", WordAttributeNormal, BodyText},
        {"and", WordAttributeNormal, BodyText},
        {"all", WordAttributeNormal, BodyText},
        { "of", WordAttributeNormal, BodyText},
        {"the", WordAttributeHeading, BodyText},
        {"animals", WordAttributeNormal, BodyText},
    };

    IndexHT *index = new IndexHT();
    index->addDoc("https://eecs440.com", words, 9);
    index->addDoc("https://eecs441.com", words, 9);

    size_t bytesRequired = index->BytesRequired();

    std::cout << bytesRequired << std::endl;

    APESEARCH::string strToFind = "the";

    hash::Tuple<APESEARCH::string, PostingList *> * entry = index->dict.Find(strToFind);

    char buffer [17];
    buffer[16] = 0;

    WordPostingList* theList = (WordPostingList *) entry->value;

    std::cout << theList->deltas.size() << std::endl;

    for(auto byte : theList->deltas){
        printf("%d ", byte);
    }

    assert(theList->deltas[1] == 0); //WordAttributeNormal
    assert(theList->deltas[3] == 1); //WordAttributeBold
    assert(theList->deltas[5] == 2); //WordAttributeHeading

    assert(theList->deltas[0] + theList->deltas[2] + theList->deltas[4] == 7); //added up deltas = abs location of 3rd the

    std::cout << std::endl;

    //assert(entry->value->bytesList == 1048);

    std::cout << entry->value->bytesList << std::endl;
}

TEST(basic_index_file_write_test){
     APESEARCH::vector<IndexEntry> words = {
        {"the", WordAttributeNormal, BodyText},
        {"cow", WordAttributeNormal, BodyText},
        {"the", WordAttributeBold, BodyText},
        {"pig", WordAttributeNormal, BodyText},
        {"and", WordAttributeNormal, BodyText},
        {"all", WordAttributeNormal, BodyText},
        { "of", WordAttributeNormal, BodyText},
        {"the", WordAttributeHeading, BodyText},
        {"animals", WordAttributeNormal, BodyText},
    };

    IndexHT *index = new IndexHT();
    index->addDoc("https://eecs440.com", words, 9);
    index->addDoc("https://eecs441.com", words, 9);

    char const *filename = "./tests/testIndexBlobFile.txt";
    IndexFile hashFile( filename, index );

}


TEST(basic_index_file_read_test){
    char const *filename = "./tests/testIndexBlobFile.txt";

    IndexFile hashFile (filename);

    const IndexBlob* blob = hashFile.Blob();
    APESEARCH::string strToFind = "the";

    const SerializedPostingList* pl = blob->Find(strToFind);
    
    std::cout << pl->Key << std::endl;

    uint8_t * ptr = (uint8_t * ) &pl->Key;
    ptr += strlen(pl->Key) + 1;


    while(ptr < (uint8_t * ) pl + pl->bytesOfPostingList)
        printf("%d ", *ptr++);

    std::cout << std::endl;

}


TEST(sync_table){
    char const *filename = "./tests/testIndexBlobLarge.txt";

    IndexFile hashFile (filename);
    const IndexBlob* blob = hashFile.Blob();

    APESEARCH::string strToFind = "the";

    const SerializedPostingList* pl = blob->Find(strToFind);
    const SerializedPostingList* notFound = blob->Find(APESEARCH::string("dfjakslfjldksa;f"));

    assert(notFound == nullptr);

    
    for(int i = 0; i < SYNCTABLESIZE; ++i){
        printf("%d %d\n", pl->syncTable[i].seekOffset, pl->syncTable[i].absoluteLoc);
    }

    assert(pl->syncTable[1].absoluteLoc == 517);
    assert(pl->syncTable[2].absoluteLoc == 1044);
    assert(pl->syncTable[3].absoluteLoc == 2082);
    assert(pl->syncTable[4].absoluteLoc == 4101);
    assert(pl->syncTable[5].absoluteLoc == 8405);



    ListIterator* itr = new ListIterator(pl);

    Post* p = itr->Seek(3012);

    assert(p->loc == 3013);
    p = itr->Next();
    assert(p->loc == 3021);

    p = itr->Seek(1000000000);

    assert(p->tData == NullPost);

    delete itr;
    
}


TEST_MAIN();