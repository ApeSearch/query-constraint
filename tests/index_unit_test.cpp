#include "../include/IndexHT.h"
#include "../include/IndexFileParser.h"
#include <iostream>

#include "../libraries/unit_test_framework/include/unit_test_framework/unit_test_framework.h"


size_t FileSize( int f )
    {
    struct stat fileInfo;
    fstat( f, &fileInfo );
    return ( size_t ) fileInfo.st_size;
    }

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

    IndexFileParser parser(filename);
    APESEARCH::string s1 = "continue";
    APESEARCH::string s2 = "%the";
    APESEARCH::string s3 = "newsletter";

    hash::Tuple<APESEARCH::string, PostingList *> * entry1 = parser.index->dict.Find(s1);
    hash::Tuple<APESEARCH::string, PostingList *> * entry2 = parser.index->dict.Find(s2);
    hash::Tuple<APESEARCH::string, PostingList *> * entry3 = parser.index->dict.Find(s3);
    hash::HashTable<APESEARCH::string, PostingList *>::iterator itr = parser.index->dict.begin();

    assert(entry1->value->posts[0]->loc == 12);
    assert(entry2->value->posts[0]->loc == 0);
    
    assert(entry3->value->posts[0]->loc == 689);
    assert(entry3->value->posts[1]->loc == 716);


    /*
    while(itr != parser.index->dict.end()){
        std::cout << itr->key << std::endl;
        for(size_t i = 0; i < itr->value->posts.size(); ++i) {
            std::cout << itr->value->posts[i]->loc << " ";
        }
        std::cout << std::endl;
        itr++;
    }*/

}


TEST(basic_encode_deltas_bytes){
     APESEARCH::vector<IndexEntry> words = {
        {"the", WordAttributeNormal, BodyText},
        {"cow", WordAttributeNormal, BodyText},
        {"the", WordAttributeNormal, BodyText},
        {"pig", WordAttributeNormal, BodyText},
        {"and", WordAttributeNormal, BodyText},
        {"all", WordAttributeNormal, BodyText},
        { "of", WordAttributeNormal, BodyText},
        {"the", WordAttributeNormal, BodyText},
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

    for(auto byte : entry->value->deltas){
        printf("%d ", byte);
    }

    std::cout << std::endl;

    printf("AttributeNormal: %d", WordAttributeNormal);
}

TEST_MAIN();