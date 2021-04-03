 
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
typedef size_t Location; // The numbering of a token
typedef size_t FileOffset; 

enum WordAttributes 
    {
        WordAttributeNormal, WordAttributeBold, WordAttributeHeading, WordAttributeLarge
    };


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
        PostingList(): posts(), synchTable(), numberOfBytes(0), numberOfPosts(0), numOfDocs(0) {}
        ~PostingList() {}

        APESEARCH::vector<Post *> posts;
        APESEARCH::vector<SynchronizationEntry> synchTable;
        size_t numberOfBytes; // size of posting list
        size_t numberOfPosts;   // Basically the number of occurnces of a particular token.
        size_t numOfDocs; // number of documents that contain this info.
        TokenType type; // Type of token ( be it eod, anchor text, url, tile, or body)

        Post *Seek( Location l );


    private:
        struct PostingListIndex //sync table
            {
            FileOffset Offset;
            Location PostLocation;
            };

        //virtual char *GetPostingList( );
    };

class WordPostingList : public PostingList
    {

    public:
        WordPostingList(): PostingList() {}
        ~WordPostingList() {}

        void appendToList(Location tokenLoc, WordAttributes attribute);

    };

class DocEndPostingList : public PostingList
    {

    public:
        DocEndPostingList(): PostingList() {}
        ~DocEndPostingList() {}

        void appendToList(Location urlLoc, size_t urlIndex);
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
        size_t   WordsInIndex,
                    DocumentsInIndex,
                    LocationsInIndex,
                    MaximumLocation;

    };