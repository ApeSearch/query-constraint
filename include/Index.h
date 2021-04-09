 
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

/*
static const uint32_t k2Exp7 = 128;
static const uint32_t k2Exp14 = 16384;
static const uint32_t k2Exp21 = (2 * 1024 * 1024);
static const uint32_t k2Exp28 = (256 * 1024 * 1024);

*/

class ISR;
class ISRWord;
class ISREndDoc;


typedef size_t Location; // The numbering of a token
typedef size_t FileOffset; 

enum WordAttributes
    {
        WordAttributeNormal, WordAttributeBold, WordAttributeHeading, WordAttributeLarge
    };


enum PostingListType
    {
        BodyText, TitleText, HeaderText, AnchorText, URL
    };

typedef union Attributes //attributes for all kinds of posting lists, only one value will be used
    {
        Attributes(WordAttributes attribute_) : attribute(attribute) {}
        Attributes(size_t urlIndex_) : urlIndex(urlIndex_) {}
        //Is a destructor needed here?

        WordAttributes attribute;
        size_t urlIndex;
    };



/* This class is only needed when flattening the in memory hashtable
class SynchronizationEntry
   {
    size_t highBitOfSeekLoc; // Relative to a posting list ( ordered )
    size_t seekOffset;  // Relative to the start of the posting list.
    size_t absoluteLoc; // Relative to every posting list.
   };

*/

class Post
    { //in memory version of post, will replace loc with deltaPrev once written to disk
    public:
        Post() : loc(), attribute(WordAttributeNormal){}
        Post(Location loc_, Attributes attribute_) : loc(loc_), attribute(attribute_) {}

        Location loc;
        Attributes attribute;
    };

class WordPost: public Post
    {
    public:
        WordPost(): Post() {}
        WordPost(FileOffset loc_, WordAttributes attribute_) : Post(loc_, attribute_) {}

    };

class EODPost: public Post
    {
    public:
        EODPost(): Post() {}
        EODPost(Location loc_, size_t urlIndex_) : Post(loc_, urlIndex_) {}
    };

class PostingList
    {
    public:

        PostingList(): posts(), numberOfBytes(0), numberOfPosts(0), numOfDocs(0) {}
        ~PostingList() {
            for(size_t i = 0; i < posts.size(); ++i)
                delete posts[i];
        }

        APESEARCH::vector<Post *> posts; //pointers to individual posts
        size_t numberOfBytes; // size of posting list
        size_t numberOfPosts;   // Basically the number of occurnces of a particular token.
        size_t numOfDocs; // number of documents that contain this info.

        Post *Seek( Location l );

        //pure virtual function to handle appending a new post to list. lastDocIndex is for 
        //word/token posts to determine absolute location based on location of last document
        virtual void appendToList(Location loc_, Attributes attribute, size_t lastDocIndex = 0) = 0;


        //virtual char *GetPostingList( );
    };

class WordPostingList : public PostingList
    {

    public:

        WordPostingList(): PostingList() {}

        void appendToList(Location loc_, Attributes attribute, size_t lastDocIndex = 0) override;

        
    };

class DocEndPostingList : public PostingList
    {

    public:

        DocEndPostingList(): PostingList() {}
        ~DocEndPostingList() {}

        void appendToList(Location loc_, Attributes attribute, size_t lastDocIndex = 0) override; 
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

        void addDoc(APESEARCH::string url, APESEARCH::vector<APESEARCH::string> text, size_t endDocLoc, PostingListType type);
        Post *goToNext( Location location ); // May need to inherit here...

        ISRWord* getWordISR ( APESEARCH::string word );
        ISREndDoc* getEndDocISR ( );

        hash::HashTable<APESEARCH::string, PostingList *> dict;
        APESEARCH::vector<APESEARCH::string> urls;
        size_t LocationsInIndex, MaximumLocation;

    };