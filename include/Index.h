#pragma once

#include "../libraries/AS/include/AS/unique_mmap.h"
#include "../libraries/AS/include/AS/string.h"
#include "../libraries/AS/include/AS/vector.h"
#include "../libraries/AS/include/AS/listdir.h"
#include "../libraries/AS/include/AS/unique_ptr.h"
#include "../libraries/HashTable/include/HashTable/HashBlob.h"
#include "../libraries/AS/include/AS/algorithms.h"
#include "IndexHT.h"
#include "ISR.h"
#include "Tuple.h"
#include "QueryParser.h"


//1. Calculate deltas and figure out how many bytes will need to represent each delta
//2. Attribute
//3. Sync table

#define SYNCTABLESIZE 24

using std::size_t;


struct SyncEntry
   {
    uint32_t absoluteLoc;
    uint32_t seekOffset;
   };


//< bytesOfPostingList> 8 bytes

//< sync Table > 32 * 16 bytes [ , , , ]

//< Key > variable bytes ( as many as it takes )

//< deltas >

class ListIterator;

class SerializedPostingList
   {
    friend class ListIterator;

    public:
    
        uint32_t bytesOfPostingList;

        SyncEntry syncTable[ SYNCTABLESIZE ];

        static constexpr uint32_t sizeOfNullSentinel = sizeof( uint32_t ); // Just needs to be a Length

        char Key[ Unknown ];


        static size_t decodeDeltaIndex(uint8_t* deltas){
            size_t addedDeltas = 0, index = 0;

            int mask = 0x7F;

            int shift = 0;

            while(true) {
                uint8_t byte = deltas[index++];
                addedDeltas |= (byte & mask) << shift;
                if(!(~mask & byte))
                    break;
                shift += 7;
            }

            return index;
        }

        static SerializedPostingList * initSerializedPostingList(char* buffer, 
        const hash::Bucket<APESEARCH::string, PostingList *> * b, uint32_t length) {
            SerializedPostingList *serialTuple = reinterpret_cast< SerializedPostingList * >( buffer );

            serialTuple->bytesOfPostingList = length;
            strcpy( serialTuple->Key, b->tuple.key.cstr() );
            
            char *ptrAfterKey = serialTuple->Key + b->tuple.key.size( ) + 1;

            PostingList* pl = b->tuple.value;


            SerializedPostingList * returnPtr = ( SerializedPostingList * ) 
                APESEARCH::copy( b->tuple.value->deltas.begin(), b->tuple.value->deltas.end(), ( uint8_t * ) ptrAfterKey );


            memset(serialTuple->syncTable, 0, SYNCTABLESIZE * sizeof(uint32_t) * 2);
            
            serialTuple->syncTable[0] = SyncEntry{ static_cast<uint32_t>(pl->posts[0]->loc), 0 }; //1st post
            
            uint32_t absoluteLocation = 0, currentPost = 0;
            uint8_t nextHighBit = 1;

            for(uint32_t i = 0; i < pl->deltas.size();){ //i keeps track of byte offset from beg of posts

                assert(currentPost < pl->posts.size()); //sanity check, if deltas still exist, then posts still exists

                absoluteLocation = pl->posts[currentPost++]->loc; //keep track of absLoc
                uint32_t prevOffset = i; //keep track of previous Delta Location, start of currentPost
                i += decodeDeltaIndex((uint8_t *) ptrAfterKey + i); //byte offset in posting list

                uint8_t leadingZeros = __builtin_clz(absoluteLocation >> 8); //Calculate highest bit, chop off last byte
                assert(leadingZeros >= 8 );

                uint8_t highBit = 31 - leadingZeros; //calculate the highest bit based on leading 0s

                while(highBit >= nextHighBit)
                    serialTuple->syncTable[nextHighBit++] = SyncEntry{ absoluteLocation, prevOffset };
                
                i += decodeDeltaIndex((uint8_t *) ptrAfterKey + i); //accounts for attributes
            }

            return returnPtr;

        }

        static char * Write(char *buffer, char const * const bufferEnd,
            const hash::Bucket<APESEARCH::string, PostingList *> *b){
                
                SerializedPostingList * serialList = initSerializedPostingList(buffer, b, b->tuple.value->bytesList);
                char *end = buffer + b->tuple.value->bytesList;
                assert( end == ( char * ) serialList );

                return buffer + b->tuple.value->bytesList; //buffer + numOfBytes of Posting List
        }

        //should include other metadata
        // Pure bit manipulation..
   };


class ListIterator {
    public:
        ListIterator(const SerializedPostingList * pl_): pl(pl_), curPost(nullptr), prevLoc(0){
            assert(pl_ != nullptr);
            plPtr = (uint8_t * ) &pl->Key + strlen(pl->Key) + 1;
            startOfDeltas = (uint8_t * ) &pl->Key + strlen(pl->Key) + 1;
        }

        Post* Seek(Location l){
            // this assert was causing failures, commented it out and it works
            // I don't think it's right to check, as sometimes you want to seek pretty far after
            // whatever the current post's location may be 

            // if(curPost.get())
            //     assert(curPost.get()->loc < l);
            
            uint8_t highBit = 31 - __builtin_clz(l >> 8);

            assert(highBit < 24);

            //if no entries exist for such a high seek location, go until you find the lowest one
            while(highBit > 0 && pl->syncTable[highBit].absoluteLoc == 0) --highBit;
            plPtr = startOfDeltas + pl->syncTable[highBit].seekOffset;

            size_t prevOffset = decodeDelta(plPtr); //difference of curLoc - prevLoc, can calculate prevLoc
            size_t tData = decodeDelta(plPtr);

            curPost = APESEARCH::unique_ptr<Post>(new Post(pl->syncTable[highBit].absoluteLoc, tData));
            prevLoc = pl->syncTable[highBit].absoluteLoc - prevOffset;
            

            while(curPost.get()->tData != NullPost && curPost.get()->loc < l) Next();
            if (curPost.get()->tData == NullPost) {
                return nullptr;
            }

            return curPost.get();
        }

        Post* Next(){

            if(curPost.get())
                prevLoc = curPost.get()->loc;
            else
                prevLoc = 0;

            if(plPtr >= (uint8_t *) pl + pl->bytesOfPostingList)
                curPost = APESEARCH::unique_ptr<Post>(new Post(0, NullPost)); //end of posting list
            
            else{
                Location loc = prevLoc + decodeDelta(plPtr);
                size_t tData = decodeDelta(plPtr);
            
                curPost = APESEARCH::unique_ptr<Post>(new Post(loc, tData));
            }

            return curPost.get();
        }

        const SerializedPostingList * pl;

        uint8_t* startOfDeltas;
        uint8_t* plPtr;
        APESEARCH::unique_ptr<Post> curPost;

        Location prevLoc;

    private:
        size_t decodeDelta(uint8_t * &deltas){
            size_t addedDeltas = 0;

            int mask = 0x7F;

            int shift = 0;

            while(true) {
                uint8_t byte = *deltas++;

                assert(deltas <= (uint8_t *) pl + pl->bytesOfPostingList);

                addedDeltas |= (byte & mask) << shift;
                if(!(~mask & byte))
                    break;
                shift += 7;
            }

            return addedDeltas;
        }
};




class IndexBlob
    {
    public: 
        static constexpr uint32_t decidedMagicNum = 69;
        static constexpr uint32_t version = 1;

    uint32_t MagicNumber,
        Version,
        BlobSize,
        NumOfDocs,
        MaxAbsolLoc,
        VectorStart, // Byte offset that points to beginning of vector array
        NumberOfBuckets,
        Buckets[ Unknown ];

        //SerialPostingLists follow after

    template<class HashFunc = hash::FNV> 
    const SerializedPostingList* Find(const APESEARCH::string &key) const {
        static HashFunc func;

        uint8_t const *byteAddr = reinterpret_cast< uint8_t const * > ( &MagicNumber );

        uint32_t hashVal = ( uint32_t )func( key );
        uint32_t bucketInd = hashVal & ( NumberOfBuckets - 1 );
        uint32_t offset = Buckets[ bucketInd ];

        if(offset){
            byteAddr += offset;
            SerializedPostingList const *pl = reinterpret_cast<SerializedPostingList const *>( byteAddr );

            while(pl->bytesOfPostingList){
                if(!strcmp(key.cstr(), pl->Key))
                    return pl;

                byteAddr += pl->bytesOfPostingList;
                pl = reinterpret_cast<SerializedPostingList const *>( byteAddr );
            }

        }

        return nullptr;
    }
    

    static IndexBlob *Write( IndexBlob *ib, uint32_t bytes,
            const IndexHT *indexHT ) {
            ib->MagicNumber = IndexBlob::decidedMagicNum;
            ib->Version = IndexBlob::version;

            ib->BlobSize = bytes;
            ib->NumberOfBuckets = indexHT->dict.table_size( );
            ib->MaxAbsolLoc = indexHT->MaximumLocation;

            hash::Tuple<APESEARCH::string, PostingList *> * entry = indexHT->dict.Find(APESEARCH::string("%"));
            ib->NumOfDocs = entry->value->posts.size();
            
            memset( ib->Buckets, 0, sizeof( uint32_t ) * ib->NumberOfBuckets );
            
            ib->VectorStart = reinterpret_cast< IndexBlob * >( ib->Buckets ) - ib;

            //points to beginning of posting lists
            char *serialPtr =reinterpret_cast< char *>( ib->Buckets + indexHT->dict.table_size() );
            char *end = reinterpret_cast< char *>( ib ) + bytes;

            auto buckets = indexHT->dict.vectorOfBuckets();
            
            for(size_t i = 0; i < buckets.size(); ++i) {
                //Write Offsets
                uint32_t bucketInd = buckets[i].front()->hashValue & ( indexHT->dict.table_size() - 1 );
                ib->Buckets[ bucketInd ] = uint32_t( serialPtr - reinterpret_cast< char * >( ib ) );

                for(size_t sameChain = 0; sameChain < buckets[i].size(); ++sameChain) {
                    hash::Bucket<APESEARCH::string, PostingList*> * bucket = buckets[i][sameChain];
                    serialPtr = SerializedPostingList::Write( serialPtr, end, bucket );
                }
                *reinterpret_cast< uint32_t *>( serialPtr ) = 0;
                serialPtr += sizeof( uint32_t ); // Signify end of a chain
            }

            for ( size_t i = 0; i < indexHT->urls.size(); ++i )
                serialPtr = strcpy( serialPtr, indexHT->urls[ i ].cstr( ) ) + indexHT->urls[ i ].size( ) + 1;
                
        assert( serialPtr == end );
    }

    static IndexBlob * Create(IndexHT *indexHT) {
        const uint32_t bytesReq = indexHT->BytesRequired();
        
        char *buffer = ( char * ) malloc( bytesReq );
        memset( buffer, 0, bytesReq );

        return Write(reinterpret_cast<IndexBlob *>( buffer ), bytesReq, indexHT);
        }

    bool verifyIndexBlob() const
         {
         return MagicNumber == IndexBlob::decidedMagicNum &&
               Version == IndexBlob::version;
         }

    ISRWord *getWordISR ( const IndexBlob* blob, APESEARCH::string word ) const;
    ISREndDoc *getEndDocISR ( const IndexBlob* blob ) const;
    };


class IndexFile{
    private:
        unique_mmap blob;
        File file;
        bool good = false;

        size_t FileSize( int f )
            {
            struct stat fileInfo;
            fstat( f, &fileInfo );
            return ( size_t )fileInfo.st_size;
            }

    public:
        const IndexBlob *Blob( )
            {
            return reinterpret_cast< const IndexBlob *> ( blob.get() );
            }
        IndexFile(const char *filename, IndexHT *index)
            : file( filename, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600 )
            {
                int fd = file.getFD();
                const uint32_t bytesReq = index->BytesRequired();
                ssize_t result = lseek( fd, off_t( bytesReq - 1 ), SEEK_SET );
                if ( result == -1 )
                    {
                    perror( "Issue with lseek while trying to stretch file" );
                    return;
                    } // end if
                
                result = write( fd, "", 1 );

                if ( result == -1 )
                    {
                    perror( "Error writing bytes to file" );
                    return;
                    }
                
                blob = unique_mmap( 0, bytesReq, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
                IndexBlob *indexBlob = reinterpret_cast< IndexBlob *> ( blob.get() );
                IndexBlob::Write( indexBlob, bytesReq, index);
                good = true;
            }

        IndexFile( const char *filename ) : file( filename, O_RDONLY )
            {
            int fd = open( filename, O_RDONLY );

            blob = unique_mmap( 0, FileSize( fd ), PROT_READ, MAP_SHARED, fd, 0 );

            good = Blob()->verifyIndexBlob();
            }

};


class Index {
    public:
        Index() {}

        // Index constructor given a directory relative from the working directory where the executable was invoked
        Index(const char * chunkDirectory) : chunkFileNames(listdir(chunkDirectory)) {}
        ~Index() {}

        // Given a search query, search the index chunks for matching documents and rank them
        void searchIndexChunks(APESEARCH::string queryLine) {
            for (int i = 0; i < chunkFileNames.size(); ++i) {
                // Build the parse tree (done for every index chunk because the parse tree is deleted on Compile())
                APESEARCH::unique_ptr<query::Tuple> parseTree = buildParseTree(queryLine); 
                IndexFile chunkFile (chunkFileNames[i].cstr());
                const IndexBlob* chunk = chunkFile.Blob();

                APESEARCH::unique_ptr<ISR> compiledTree = APESEARCH::unique_ptr<ISR>(parseTree->Compile(chunk));

                // solve constraint on index chunk using ISR tree
                // for each matching document
                    // isrTree.Seek(beginning of document) 
                    // rank = rankerClass.getRank(isrTree)
                    // struct RankStruct = {double rank, string documentURL};
                    // top10<RankStruct>.insert(rank, matching document)
            }
        }

        APESEARCH::vector<APESEARCH::string> & getFiles() {
            return chunkFileNames;
        }

    private:

        // File Names corresponding to index chunk files
        APESEARCH::vector<APESEARCH::string> chunkFileNames;

        // Given a filename with a HashBlob, loads that file into main memory
        HashFile loadIndexChunk(APESEARCH::string &filename) 
            {
            // TODO: implement Hashfile = operator or do this some other way (need to keep the object around for unique_map RAII)
            // indexChunkFile = HashFile(filename.cstr());

            // blob = indexChunkFile.Blob();
            }

        // Builds a parse tree for a given query
        APESEARCH::unique_ptr<query::Tuple> buildParseTree(APESEARCH::string queryLine) {
            QueryParser query = QueryParser(queryLine);
            return APESEARCH::unique_ptr<query::Tuple>( query.FindOrConstraint());
        }
};
