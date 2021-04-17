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



class ISR;
class ISRWord;
class ISREndDoc;

class SerializedPostingList;
class IndexBlob;


typedef size_t Location; // The numbering of a token
typedef size_t FileOffset; 

using std::size_t;

enum WordAttributes
    {
        WordAttributeNormal, WordAttributeBold, WordAttributeHeading
    };


enum PostingListType
    {
        BodyText, TitleText, AnchorText, URL
    };


struct IndexEntry {
    APESEARCH::string word;
    WordAttributes attribute;
    PostingListType plType;
};

class Post
    { //in memory version of post, will replace loc with deltaPrev once written to disk
    public:
        Post() : loc(), tData(WordAttributeNormal){}
        Post(Location loc_, size_t tData_) : loc(loc_), tData(tData_) {}

        Location loc;
        size_t tData;
    };

class WordPost: public Post
    {
    public:
        WordPost(): Post() {}
        WordPost(Location loc_, size_t attribute_) : Post(loc_, attribute_) {}

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

        PostingList(): posts(), deltas(), bytesList(0), calcBytes(false) {}
        ~PostingList() {
            for (int i = 0; i < posts.size(); ++i) 
                delete posts[i];
        }

        APESEARCH::vector<Post *> posts; //pointers to individual posts
        APESEARCH::vector<uint8_t> deltas;
        uint32_t bytesList;
        bool calcBytes; //has the number of bytes already been calculated?

        
        Post *Seek( Location l );

        //pure virtual function to handle appending a new post to list. lastDocIndex is for 
        //word/token posts to determine absolute location based on location of last document
        virtual void appendToList(Location loc_, size_t tData, size_t lastDocIndex = 0) = 0;
        virtual uint32_t bytesRequired( const APESEARCH::string &key ) = 0;


        //virtual char *GetPostingList( );
    };

class WordPostingList : public PostingList
    {

    public:

        WordPostingList(): PostingList() {}

        uint32_t bytesRequired( const APESEARCH::string &key );

        void appendToList(Location loc_, size_t attribute, size_t lastDocIndex = 0) override;

        
    };

class DocEndPostingList : public PostingList
    {

    public:

        DocEndPostingList(): PostingList() {}

        uint32_t bytesRequired( const APESEARCH::string &key );

        void appendToList(Location loc_, size_t urlIndex, size_t lastDocIndex = 0) override; 
    };

class IndexHT
    {

    public:

        IndexHT();
        ~IndexHT();

        void addDoc(APESEARCH::string url, APESEARCH::vector<IndexEntry> &text, size_t endDocLoc);
        Post *goToNext( Location location ); // May need to inherit here...

        uint32_t BytesRequired();

        ISRWord *getWordISR ( APESEARCH::string word );
        ISREndDoc *getEndDocISR ( );
        
        hash::HashTable<APESEARCH::string, PostingList *> dict;
        APESEARCH::vector<APESEARCH::string> urls;
        size_t LocationsInIndex, MaximumLocation, numDocs;
        size_t bytes;
        bool calcBytes;

    };