#pragma once
#include "IndexHT.h"
#include "../libraries/AS/include/AS/string.h"
#include "../libraries/AS/include/AS/vector.h"
#include "../libraries/AS/include/AS/listdir.h"
#include "../libraries/HashTable/include/HashTable/HashBlob.h"


//1. Calculate deltas and figure out how many bytes will need to represent each delta
//2. Attribute
//3. Sync table

class Post {
    public:
        // I just threw the 8 in there so this would compile
        uint8_t delta[8];
        Attributes attribute;
        
};


class SerializedPostingList
   {

    public:
        size_t bytesForSyncTable;
        char syncTable[ Unknown ];
   };



class Index{
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
