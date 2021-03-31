
#pragma onece

#ifndef INDEX_H_AS
#define INDEX_H_AS

typedef size_t Location; // The numbering of a token
typedef size_t FileOffset;

class PostingList
    {
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
                ISR *OpenISR(char token);
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

        Index() :  chunk(), WordsInIndex(0), DocumentsInIndex(0), LocationsInIndex(0), MaximumLocation(0) {}

        std::pair<Location, Post *> findPost(  )
        Post *goToNext( Location location ); // May need to inherit here...



        //ISRWord *OpenISRWord( std::string word );

        //ISREndDoc *OpenISREndDoc( );
    };

#endif INDEX_H_AS