
#pragma onece

#ifndef INDEX_H_AS
#define INDEX_H_AS
#include "Token.h"

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

class SynchronizationEntry
   {
    size_t hightBitOfSeekLoc; // Relative to a posting list ( ordered )
    size_t seekOffset;  // Relative to the start of the posting list.
    size_t absoluteLoc; // Relative to every posting list.
   };

class Post;

class PostingList
    {
    // Represent the syncrhonoziation table
    size_t numberOfBytes; // size of posting list
    std::vector<SynchronizationEntry> synchTable;
    size_t numberOfPosts;   // Basically the number of occurnces of a particular token.
    size_t numOfDocs; // number of documents that contain this info.
    TokenType type; // Type of token ( be it eod, anchor text, url, tile, or body)
    public:
        virtual Post *Seek( Location l ); //add up deltas until location reached

    private:
        struct PostingListIndex //sync table
            {
            FileOffset Offset;
            Location PostLocation;
            };

        virtual char *GetPostingList( );
    };

enum WordAttributes 
    {
        WordAttributeBold, WordAttributeHeading, WordAttributeLarge
    };

enum DocumentAttributes 
    {
        DocAttributeURL, DocAttributeNumWords, DocAttributeTitle, DocAttributeBody, DocAttributeAnchor
    };

typedef union Attributes
    {
    WordAttributes Word;
    DocumentAttributes Document;
    };

class Post
    {
    PostingList *list; // Pointer to posting list in which it comes from...
    public:
        FileOffset deltaPrev;
    };

class WordPost: Post
    {

    };

class EODPost: Post
    {

    };

class Sentinel: Post
    {
        
    };

class Index 
    {
    

    public:

    };

class IndexHT
    {

    class WordPostingList : public PostingList
        {

        public:
            Post *Seek ( Location l ) override;

        private:
            Post* post;
        };

    class Dictionary
        {
            public:
                Location GetNumberOfWords();
                Location GetNumberOfUniqueWords();
                Location GetNumberOfDocuments();
        };
    hash::HashTable<const char *, PostingList *> chunk;
    std::vector<std::string> urls;
    public:
        size_t   WordsInIndex,
                 DocumentsInIndex,
                 LocationsInIndex,
                 MaximumLocation;

        IndexHT() :  WordsInIndex(0), DocumentsInIndex(0), LocationsInIndex(0), MaximumLocation(0) {}

        std::pair<Location, Post *> findPost(  );
        Post *goToNext( Location location ); // May need to inherit here...



        //ISRWord *OpenISRWord( std::string word );

        //ISREndDoc *OpenISREndDoc( );
    };

#endif INDEX_H_AS