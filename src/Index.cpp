#include "../include/Index.h"
#include "../include/Ranker.h"

ISRWord *IndexBlob::getWordISR ( APESEARCH::string word ) const
    {
        const SerializedPostingList* entry = Find(word);
        return new ISRWord(new ListIterator(entry), this, word);
    }
ISREndDoc *IndexBlob::getEndDocISR ( ) const
    {
        const SerializedPostingList* entry = Find(APESEARCH::string("%"));
        return new ISREndDoc(new ListIterator(entry), this);
    }


APESEARCH::vector<APESEARCH::string> IndexBlob::getUrls( ) const
    {
        APESEARCH::vector<APESEARCH::string> vecUrls;

        char * ptr = (char *) this + VectorStart;
        
        while(ptr < (char *) this + BlobSize){
            auto url = APESEARCH::string(ptr);
            ptr += url.size() + 1;

            vecUrls.emplace_back(url);
        }

        return vecUrls;
    }

void Index::searchIndexChunks(APESEARCH::string queryLine) {
    for (int i = 0; i < chunkFileNames.size(); ++i) {
        // Build the parse tree (done for every index chunk because the parse tree is deleted on isr->Compile())
        IndexFile chunkFile (chunkFileNames[i].cstr());
        const IndexBlob* chunk = chunkFile.Blob();

        // Flatten, 
        // create Ranker(flattened)
        Ranker ranker(chunk, queryLine);
    }
}