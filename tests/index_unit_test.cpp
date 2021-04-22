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


TEST(build_with_file){
    const char *filename = "./tests/indexFiles/indexFile1.txt";

    IndexFileParser parser;
    parser.writeFile(filename);


    APESEARCH::string s1 = "continue";
    APESEARCH::string s2 = "$the";
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

    while(itr != parser.index->dict.end()){
        std::cout << itr->key << std::endl;
        for(size_t i = 0; i < itr->value->posts.size(); ++i) {
            std::cout << itr->value->posts[i]->loc << "-" << itr->value->posts[i]->tData << ' ';
        }
        std::cout << std::endl;
        itr++;
    }
    
    size_t bytesRequired = parser.index->BytesRequired();

    size_t bytes = entry4->value->bytesRequired("the");

    WordPostingList * theList = (WordPostingList * )entry4->value;

    char const *filenameo = "./tests/testIndexBlobLarge.txt";
    IndexFile hashFile( filenameo, parser.index.get() );

    std::cout << "PASS" << std::endl;

}


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

    APESEARCH::unique_ptr<IndexHT> index = APESEARCH::unique_ptr<IndexHT>(new IndexHT());

    APESEARCH::vector<AnchorText> aText;

    index->addDoc("https://eecs440.com", words, aText, 9);
    index->addDoc("https://eecs441.com", words, aText, 9);

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


    std::cout << std::endl;

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
    
    APESEARCH::unique_ptr<IndexHT> index = APESEARCH::unique_ptr<IndexHT>(new IndexHT());

    APESEARCH::vector<AnchorText> aText;

    index->addDoc("https://eecs440.com", words, aText, 9);
    index->addDoc("https://eecs441.com", words, aText, 9);

    char const *filename = "./tests/testIndexBlobFile.txt";
    IndexFile hashFile( filename, index.get() );

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
    assert(pl->syncTable[3].absoluteLoc == 2081);
    assert(pl->syncTable[4].absoluteLoc == 4099);
    assert(pl->syncTable[5].absoluteLoc == 8403);



    WordListIterator* itr = new WordListIterator(pl);

    Post* p = itr->Seek(3012);

    assert(p->loc == 3019);
    p = itr->Next();
    assert(p->loc == 3026);

    p = itr->Seek(1000000000);

    assert(p == nullptr);

    delete itr;
    
}


TEST(find_urls){
    //char const *filename = "./tests/indexChunks/chunk0.ape";
    char const *filename = "./tests/testIndexBlobLarge.txt";

    IndexFile hashFile (filename);
    const IndexBlob* blob = hashFile.Blob();

    auto urls = blob->getUrls();

    const char * c = (char *) blob + blob->VectorStart;
    
    assert(urls[0] == "www.monkey.com");
    assert(urls[1] == "www.monkey1.com");
    assert(urls[2] == "www.monkey2.com");
    assert(urls[3] == "www.monkey3.com");

}

TEST(docEnd_indicies){
    char const *filename = "./tests/testIndexBlobFile.txt";

    IndexFile hashFile (filename);
    const IndexBlob* blob = hashFile.Blob();

    APESEARCH::string strToFind = "%";

    const SerializedPostingList* pl = blob->Find(strToFind);

    EndDocListIterator itr(pl);

    Post* p = itr.Next();

    assert(p->tData == 0);
    p = itr.Next();

    assert(p->tData == 1);
    p = itr.Next();

    assert(p == nullptr);
}

TEST_MAIN();