#pragma once

#include "../libraries/AS/include/AS/string.h"
#include "../libraries/AS/include/AS/vector.h"
#include "../libraries/AS/include/AS/listdir.h"
#include "../libraries/HashTable/include/HashTable/HashBlob.h"
#include "../libraries/AS/include/AS/algorithms.h"
#include "IndexHT.h"


//1. Calculate deltas and figure out how many bytes will need to represent each delta
//2. Attribute
//3. Sync table

#define SYNCTABLESIZE 32

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
        //size_t absoluteLoc;

        SyncEntry syncTable[ SYNCTABLESIZE ];

        static constexpr size_t sizeOfNullSentinel = sizeof( size_t ); // Just needs to be a Length

        char Key[ Unknown ];

        static SerializedPostingList * initSerializedPostingList(char* buffer, 
        const Bucket<APESEARCH::string, PostingList *> * b, size_t length) {
            SerializedPostingList *serialTuple = reinterpret_cast< SerializedPostingList * >( buffer ); 

            serialTuple->bytesOfPostingList = length;

            char *ptrAfterKey = strcpy( serialTuple->Key, b->tuple.key.cstr() ) + strlen( b->tuple.key.cstr() ) + 1;
            
            serialTuple->syncTable[0] = SyncEntry { b->tuple.value->deltas[0], 0 };
            
            size_t absoluteLocation = b->tuple.value->deltas[0], currentDelta = 1;
            uint8_t highBit = 1;

            //while(highBit < 256){
            //    
            //}
            
            APESEARCH::copy( b->tuple.value->deltas.begin(), b->tuple.value->deltas.end(), ( uint8_t * ) ptrAfterKey );
            
        }

        static char * Write(char *buffer, char const * const bufferEnd,
            const Bucket<APESEARCH::string, PostingList *> *b){
                SerializedPostingList * serialList = initSerializedPostingList(buffer, b, b->tuple.value->bytesList);

                return buffer + b->tuple.value->bytesList;
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

    static IndexBlob *Write( IndexBlob *ib, size_t bytes,
            const IndexHT *indexHT ) {
                ib->MagicNumber = IndexBlob::decidedMagicNum;
                ib->Version = IndexBlob::version;

                ib->BlobSize = bytes;
                ib->NumberOfBuckets = indexHT->dict.table_size();
                ib->NumOfDocs = indexHT->dict.numOfLinkedLists();

                memset( ib->Buckets, 0, sizeof( size_t ) * ib->NumberOfBuckets );

                ib->VectorStart = reinterpret_cast< IndexBlob * >( ib->Buckets ) - ib;

                //points to beginning of posting lists
                char *serialPtr =reinterpret_cast< char *>( ib->Buckets + indexHT->dict.table_size() );
                char *end = reinterpret_cast< char *>( ib ) + bytes;

                APESEARCH::vector< APESEARCH::vector< hash::Bucket< APESEARCH::string, PostingList*> *> > buckets = indexHT->dict.vectorOfBuckets();

                for(size_t i = 0; i < buckets.size(); ++i){
                    for(size_t sameChain = 0; sameChain < buckets[i].size(); ++sameChain){
                        hash::Bucket<APESEARCH::string, PostingList*> * bucket = buckets[i][sameChain];
                        
                        
                    }
                }

            }

    static IndexBlob * Create(IndexHT *indexHT) {
        const size_t bytesReq = indexHT->BytesRequired();

        char *buffer = ( char * ) malloc( bytesReq );
        memset( buffer, 0, bytesReq );

        return Write(reinterpret_cast<IndexBlob *>( buffer ), bytesReq, indexHT);
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
