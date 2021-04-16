#pragma once

#include "../libraries/AS/include/AS/unique_mmap.h"
#include "../libraries/AS/include/AS/string.h"
#include "../libraries/AS/include/AS/vector.h"
#include "../libraries/AS/include/AS/listdir.h"
#include "../libraries/HashTable/include/HashTable/HashBlob.h"
#include "../libraries/AS/include/AS/algorithms.h"
#include "IndexHT.h"


//1. Calculate deltas and figure out how many bytes will need to represent each delta
//2. Attribute
//3. Sync table

#define SYNCTABLESIZE 64

using std::size_t;


struct SyncEntry
   {
    size_t absoluteLoc;
    size_t seekOffset;
   };


//< bytesOfPostingList> 8 bytes

//< sync Table > 32 * 16 bytes [ , , , ]

//< Key > variable bytes ( as many as it takes )

//< deltas >

class SerializedPostingList
   {
    public:
        size_t bytesOfPostingList;

        SyncEntry syncTable[ SYNCTABLESIZE ];

        static constexpr size_t sizeOfNullSentinel = sizeof( size_t ); // Just needs to be a Length

        char Key[ Unknown ];

        static SerializedPostingList * initSerializedPostingList(char* buffer, 
        const hash::Bucket<APESEARCH::string, PostingList *> * b, size_t length) {
            SerializedPostingList *serialTuple = reinterpret_cast< SerializedPostingList * >( buffer );

            serialTuple->bytesOfPostingList = length;
            strcpy( serialTuple->Key, b->tuple.key.cstr() );
            
            char *ptrAfterKey = serialTuple->Key + b->tuple.key.size( ) + 1;

            PostingList* pl = b->tuple.value;

            memset(serialTuple->syncTable, 0, SYNCTABLESIZE * sizeof(size_t) * 2);
            
            serialTuple->syncTable[0] = SyncEntry{ pl->posts[0]->loc, 0 };
            
            size_t absoluteLocation = pl->posts[0]->loc, currentPost = 1;
            uint8_t nextHighBit = 1;

            for(; currentPost < pl->posts.size(); ++currentPost){
                absoluteLocation += pl->posts[currentPost]->loc;

                uint8_t highBit = absoluteLocation >> 24;

                while(highBit >= nextHighBit)
                    serialTuple->syncTable[nextHighBit++] = SyncEntry{ absoluteLocation, currentPost };
            }

            return ( SerializedPostingList * ) APESEARCH::copy( b->tuple.value->deltas.begin(), b->tuple.value->deltas.end(), ( uint8_t * ) ptrAfterKey );
            //return  SerializedPostingList * ( endOfPostingList );

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




class IndexBlob
    {
    public: 
        static constexpr size_t decidedMagicNum = 69;
        static constexpr size_t version = 1;

    size_t MagicNumber,
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
        size_t bucketInd = hashVal & ( NumberOfBuckets - 1 );
        size_t offset = Buckets[ bucketInd ];

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
    

    static IndexBlob *Write( IndexBlob *ib, size_t bytes,
            const IndexHT *indexHT ) {
            ib->MagicNumber = IndexBlob::decidedMagicNum;
            ib->Version = IndexBlob::version;

            ib->BlobSize = bytes;
            ib->NumberOfBuckets = indexHT->dict.table_size( );
            ib->MaxAbsolLoc = indexHT->MaximumLocation;

            hash::Tuple<APESEARCH::string, PostingList *> * entry = indexHT->dict.Find(APESEARCH::string("%"));
            ib->NumOfDocs = entry->value->posts.size();
            
            memset( ib->Buckets, 0, sizeof( size_t ) * ib->NumberOfBuckets );
            
            ib->VectorStart = reinterpret_cast< IndexBlob * >( ib->Buckets ) - ib;

            //points to beginning of posting lists
            char *serialPtr =reinterpret_cast< char *>( ib->Buckets + indexHT->dict.table_size() );
            char *end = reinterpret_cast< char *>( ib ) + bytes;

            APESEARCH::vector< APESEARCH::vector< hash::Bucket< APESEARCH::string, PostingList*> *> > buckets = indexHT->dict.vectorOfBuckets();
            
            for(size_t i = 0; i < buckets.size(); ++i) {
                //Write Offsets
                size_t bucketInd = buckets[i].front()->hashValue & ( indexHT->dict.table_size() - 1 );
                ib->Buckets[ bucketInd ] = size_t( serialPtr - reinterpret_cast< char * >( ib ) );


                for(size_t sameChain = 0; sameChain < buckets[i].size(); ++sameChain) {
                    hash::Bucket<APESEARCH::string, PostingList*> * bucket = buckets[i][sameChain];
                    serialPtr = SerializedPostingList::Write( serialPtr, end, bucket );
                }
                *reinterpret_cast< std::size_t *>( serialPtr ) = 0;
                serialPtr += sizeof( size_t ); // Signify end of a chain
            }

            for ( size_t i = 0; i < indexHT->urls.size(); ++i )
                serialPtr = strcpy( serialPtr, indexHT->urls[ i ].cstr( ) ) + indexHT->urls[ i ].size( ) + 1;
                

        std::cout << (char * ) ib + bytes - serialPtr << std::endl;
        assert( serialPtr == end );
    }

    static IndexBlob * Create(IndexHT *indexHT) {
        const size_t bytesReq = indexHT->BytesRequired();
        
        char *buffer = ( char * ) malloc( bytesReq );
        memset( buffer, 0, bytesReq );

        return Write(reinterpret_cast<IndexBlob *>( buffer ), bytesReq, indexHT);
        }

    bool verifyIndexBlob() const
         {
         return MagicNumber == IndexBlob::decidedMagicNum &&
               Version == IndexBlob::version;
         }
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
                const std::size_t bytesReq = index->BytesRequired();
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

        // Probably given an ISR, searches for matches in each index chunk we have
        void searchIndexChunks() {
            for (int i = 0; i < chunkFileNames.size(); ++i) {
                loadIndexChunk(chunkFileNames[i]);

                // TODO: actually search the whole gd thing
            }
        }

    private:
        // TODO: implement HashFile default constructor
        // HashFile indexChunkFile;
        const HashBlob *blob;

        // File Names corresponding to index chunk files
        APESEARCH::vector<APESEARCH::string> chunkFileNames;
        
        // Given a filename with a HashBlob, loads that file into main memory
        void loadIndexChunk(APESEARCH::string &filename) 
            {
            // TODO: implement Hashfile = operator or do this some other way (need to keep the object around for unique_map RAII)
            // indexChunkFile = HashFile(filename.cstr());

            // blob = indexChunkFile.Blob();
            }
};
