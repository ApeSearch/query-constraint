
#pragma once

#ifndef INDEX_H_AS
#define INDEX_H_AS
#include "Token.h"
#include "../libraries/AS/include/AS/vector.h"
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

enum DocumentAttributes 
    {
        DocAttributeURL, DocAttributeNumWords, DocAttributeTitle, DocAttributeBody, DocAttributeAnchor
    };

typedef union Attributes
{
    WordAttributes Word;
    DocumentAttributes Document;
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
        Post(FileOffset _deltaPrev, Attributes _attribute) : deltaPrev(_deltaPrev), attribute(_attribute) {}

        FileOffset deltaPrev;
        Attributes attribute;
    };

class WordPost: public Post
    {
    public:
        WordPost(FileOffset deltaPrev, Attributes attribute) : Post(deltaPrev, attribute) {}
    };

class EODPost: public Post
    {
    public:
        EODPost(FileOffset deltaPrev, Attributes attribute) : Post(deltaPrev, attribute) {}
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
        APESEARCH::vector<Post> posts;
        size_t numberOfBytes; // size of posting list
        std::vector<SynchronizationEntry> synchTable;
        size_t numberOfPosts;   // Basically the number of occurnces of a particular token.
        size_t numOfDocs; // number of documents that contain this info.
        TokenType type; // Type of token ( be it eod, anchor text, url, tile, or body)

        virtual Post *Seek( Location l );

    private:
        struct PostingListIndex //sync table
            {
            FileOffset Offset;
            Location PostLocation;
            };

        virtual char *GetPostingList( );
    };

class WordPostingList : public PostingList
    {

    public:
        Post *Seek ( Location l ) override;

    private:
        Post* post;
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
        Post *goToNext( Location location ); // May need to inherit here...



        //ISRWord *OpenISRWord( std::string word );

        //ISREndDoc *OpenISREndDoc( );

    private:
        hash::HashTable<const char *, PostingList *> chunk;
        std::vector<std::string> urls;
        size_t   WordsInIndex,
                    DocumentsInIndex,
                    LocationsInIndex,
                    MaximumLocation;

    };

#endif INDEX_H_AS