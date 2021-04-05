 
#pragma once

#include "Token.h"
#include "../libraries/AS/include/AS/string.h"
#include "../libraries/AS/include/AS/vector.h"
#include "../libraries/AS/include/AS/unique_ptr.h"
#include "HashTable.h"

/*
 * 
 * The Index will now be a hash table like structure that ISRs can only interact with methods that
 *  expose the functionality of the Index. That is, perhaps having a pointer to a posting list, you can
 *  basically use that to find the next post in a posting list.
 *  
 * 
 * 
 * 
 * 
 * 
 * 
 */


//Constants to use for decoding and encoding deltas
//Variable length encodings


static const uint32_t k2Exp7 = 128;
static const uint32_t k2Exp14 = 16384;
static const uint32_t k2Exp21 = (2 * 1024 * 1024);
static const uint32_t k2Exp28 = (256 * 1024 * 1024);



typedef size_t Location; // The numbering of a token
typedef size_t FileOffset; 

enum WordAttributes : uint8_t
    {
        WordAttributeNormal, WordAttributeBold, WordAttributeHeading, WordAttributeLarge
    };

enum PostingListType : uint8_t
    {
        BodyText, TitleText, HeaderText, AnchorText, URL
    };

typedef union WordPostEntry
    {
        WordAttributes attribute;
        uint8_t delta;
    };

typedef uint8_t EODPostEntry;

class SynchronizationEntry
   {
    size_t highBitOfSeekLoc; // Relative to a posting list ( ordered )
    size_t seekOffset;  // Relative to the start of the posting list.
    size_t absoluteLoc; // Relative to every posting list.
   };

class Post
    {
    public:
        Post() : deltaPrev() {}
        Post(FileOffset deltaPrev_) : deltaPrev(deltaPrev_) {}

        FileOffset deltaPrev;
    };

class WordPost: public Post
    {
    public:
        WordPost(): Post() {}
        WordPost(FileOffset deltaPrev_, WordAttributes attribute_) : Post(deltaPrev_), attribute(attribute_) {}

        WordAttributes attribute;
    };

class EODPost: public Post
    {
    public:
        EODPost(): Post() {}
        EODPost(FileOffset deltaPrev_, size_t urlIndex_) : Post(deltaPrev_), urlIndex(urlIndex) {}

        size_t urlIndex;
    };


class Sentinel: public Post
    {
    public:
        Sentinel();
    };

class PostingList
    {
    public:
        // Represent the sync table
        PostingList(): synchTable(), numberOfBytes(0), numberOfPosts(0), numOfDocs(0) {}
        ~PostingList() {}

        APESEARCH::vector<SynchronizationEntry> synchTable;
        size_t numberOfBytes; // size of posting list
        size_t numberOfPosts;   // Basically the number of occurnces of a particular token.
        size_t numOfDocs; // number of documents that contain this info.
        PostingListType type; // Type of token ( be it eod, anchor text, url, tile, or body)

        Post *Seek( Location l );


    private:
        //virtual char *GetPostingList( );
    };

class WordPostingList : public PostingList
    {

    public:

        APESEARCH::vector<WordPostEntry> posts;

        WordPostingList(): PostingList() {}
        ~WordPostingList() {}

        void appendToList(Location tokenLoc, WordAttributes attribute);

    };

class DocEndPostingList : public PostingList
    {

    public:
        APESEARCH::vector<Post *> posts;

        DocEndPostingList(): PostingList() {}
        ~DocEndPostingList() {}

        //Appends to DocEndPostingList and returns the previous
        //Document's absolute location for calculating offset
        Location appendToList(Location urlLoc, size_t urlIndex); 
    };

class Index 
    {
    

    };

class IndexHT
    {

    public:
        class Dictionary
            {
                public:
                    Location GetNumberOfWords();
                    Location GetNumberOfUniqueWords();
                    Location GetNumberOfDocuments();
            };
        IndexHT();
        ~IndexHT();

        std::pair<Location, Post *> findPost(  );
        void addDoc(APESEARCH::string url, APESEARCH::vector<APESEARCH::string> &bodyText, size_t endDocLoc);
        Post *goToNext( Location location ); // May need to inherit here...

        //ISRWord *OpenISRWord( APESEARCH::string word );
        //ISREndDoc *OpenISREndDoc( );

    // private:
        hash::HashTable<const char *, WordPostingList *> chunk;
        APESEARCH::unique_ptr<DocEndPostingList> docEndList;
        APESEARCH::vector<APESEARCH::string> urls;
        size_t LocationsInIndex, MaximumLocation;

    };