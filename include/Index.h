#pragma once
#include "IndexHT.h"
#include "../libraries/AS/include/AS/string.h"
#include "../libraries/HashTable/include/Hashtable/HashBlob.h"


//1. Calculate deltas and figure out how many bytes will need to represent each delta
//2. Attribute
//3. Sync table

class Post {
    public:
        uint8_t[] delta;
        Attributes attribute;
        
}


class SerializedPostingList
   {

    public:
        size_t bytesForSyncTable;
        char syncTable[ Unknown ];


   };



class Index{
    public:
        Index() {
            // read directory, get filenames of all the index chunks

        }

        ~Index() {}

        void getIndexChunk(const char* filename) 
            {
            indexChunkFile = hashFile(filename);
            hb = hashFile.Blob();
            }

    private:
        HashFile indexChunkFile;
        HashBlob *hb;
        
}




