#include "../include/Index.h"
#include "../include/Ranker.h"

ISRWord *IndexBlob::getWordISR ( APESEARCH::string word ) const
    {
        const SerializedPostingList* entry = Find(word);
        return new ISRWord(new WordListIterator(entry), this, word);
    }
ISREndDoc *IndexBlob::getEndDocISR ( ) const
    {
        const SerializedPostingList* entry = Find(APESEARCH::string("%"));
        return new ISREndDoc(new EndDocListIterator(entry), this);
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
/*
// Procedural Version
template<class Comparator>
Pair ** insertSortN( Pair **pairArray, Pair **pairValidEnd, Pair ** pairTrueEnd, Pair *tuple, Comparator comp )
   {
   assert( pairValidEnd != pairTrueEnd );
   Pair **itr = pairValidEnd;
   *itr = tuple;
   // while tuple > currentVal
   for ( ;itr != pairArray && comp( *itr, *( itr - 1 ) ); --itr )
      swap( *itr, *(itr - 1) );
   
   return pairValidEnd + 1 == pairTrueEnd ? pairValidEnd : pairValidEnd + 1;
   } // end inserSortN()
*/
void Index::searchIndexChunks(APESEARCH::string queryLine) {
    // create start timestamp
    /*std::vector< std::future< APESEARCH::vector<RankedEntry> > > futureObjs;
    for (int i = 0; i < chunkFileNames.size(); ++i) {
        
        // Build the parse tree (done for every index chunk because the parse tree is deleted on isr->Compile())
        IndexFile chunkFile (chunkFileNames[i].cstr());
        const IndexBlob* chunk = chunkFile.Blob();

        auto func = [ rank{ Ranker( chunk, queryLine ) } ]( ) -> APESEARCH::vector<RankedEntry> mutable { return rank.getTopTen( );  };
        auto obj = threadsPool.submit(func);
        futureObjs.emplace_back( std::move( obj ) );

        if (futureObjs.size() > 200)  
            {
            for (auto& entry : futureObjs )
                {
                APESEARCH::vector< RankedEntry > results( entry.get( ) ); // will block until returned
                if ( !results.empty() )
                    {
                    for(size_t i = 0; i < results.size(); ++i)
                        {
                        
                        if(results[i].rank <= topTen.back().rank)
                            continue;

                        APESEARCH::swap(results[i], topTen.back());

                        for(size_t j = topTen.size() - 1; j > 0 && topTen[j - 1].rank < topTen[j].rank; --j)
                            APESEARCH::swap(topTen[j], topTen[j - 1]);
                        }
                    }
                }
            
            // if timestamp > 60 seconds, break
            futureObjs = std::vector< std::future< APESEARCH::vector<RankedEntry> > >( );
            }
    }*/
}