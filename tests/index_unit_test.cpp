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

TEST(build_with_file)
    {
    IndexHT* index = new IndexHT();
    const char *filename = "/Users/paulzhang/Desktop/eecs440/ApeSearch/query-constraint/tests/indexFiles/indexFile1.txt";

    IndexFileParser parser(filename);

    delete index;
    }

TEST_MAIN();