#include "../include/Index.h"
#include "../include/Ranker.h"

#include <pthread.h>

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

pthread_mutex_t resultsLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queueLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t notEmpty;

// queue<const char *> fileQueue

// thread function (void * query)
/* {
    lock queuelock
    pop chunkFile name
    unlock queuelock

    create indexfile
    get indexblob

    Ranker ranker(blob, query)
    vector<result> chunkResults = ranker.getTopTen()

    lock resultsLock
    sort chunkResults into index top 10
    unlock resultslock
   } */


void * rankChunk(void * arg){
    struct rankArgs * args = (struct rankArgs * ) arg;

    Index* index = args->index;
    APESEARCH::string query = args->query;

    pthread_mutex_lock(&queueLock);
    while(index->threadQueue.empty())
        pthread_cond_wait(&notEmpty, &queueLock);

    APESEARCH::string fileName = index->threadQueue.front();
    index->threadQueue.pop();
    pthread_mutex_unlock(&queueLock);

    IndexFile chunkFile(fileName.cstr());
    const IndexBlob* chunk = chunkFile.Blob();

    assert(chunk->verifyIndexBlob());

    Ranker ranker(chunk, query);

    APESEARCH::vector<RankedEntry> results = ranker.getTopTen();

    pthread_mutex_lock(&resultsLock);
    APESEARCH::vector<RankedEntry> &topTen = index->topTen;

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

    pthread_mutex_unlock(&resultsLock);
}


void Index::searchIndexChunks(const char * queryLine) {


    pthread_t threads[NUM_THREADS];

    struct rankArgs * args = (struct rankArgs * ) malloc(sizeof(this) + sizeof(&queryLine));
    
    args->index = this;
    args->query = queryLine;
    
    for(int i = 0; i < NUM_THREADS; ++i)
        pthread_create(&threads[i], NULL, rankChunk, (void *) args);

    // create start timestamp
    // std::vector< std::future< APESEARCH::vector<RankedEntry> > > futureObjs;
    for (int i = 0; i < chunkFileNames.size(); ++i) {
        // add filenames to queue

        pthread_mutex_lock(&queueLock);
        threadQueue.push(chunkFileNames[i]);
        pthread_cond_signal(&notEmpty);
        pthread_mutex_unlock(&queueLock);

        // Build the parse tree (done for every index chunk because the parse tree is deleted on isr->Compile())
        // IndexFile chunkFile (chunkFileNames[i].cstr());
        // const IndexBlob* chunk = chunkFile.Blob();

        // auto func = [ rank{ Ranker( chunk, queryLine )}, topTen ]( ) -> APESEARCH::vector<RankedEntry> mutable { rank.getTopTen( topTen );  };
        // pthread_create(&(threadPool[i]), nullptr, &func, nullptr);
        // auto obj = threadsPool.submit(func);
        // futureObjs.emplace_back( std::move( obj ) );

        // if (futureObjs.size() > 200)  
        //     {
        //     for (auto& entry : futureObjs )
        //         {
        //         APESEARCH::vector< RankedEntry > results( entry.get( ) ); // will block until returned
        //         if ( !results.empty() )
        //             {
        //             for(size_t i = 0; i < results.size(); ++i)
        //                 {
                        
        //                 if(results[i].rank <= topTen.back().rank)
        //                     continue;

        //                 APESEARCH::swap(results[i], topTen.back());

        //                 for(size_t j = topTen.size() - 1; j > 0 && topTen[j - 1].rank < topTen[j].rank; --j)
        //                     APESEARCH::swap(topTen[j], topTen[j - 1]);
        //                 }
        //             }
        //         }
            
        //     // if timestamp > 60 seconds, break
        //     futureObjs = std::vector< std::future< APESEARCH::vector<RankedEntry> > >( );
        //     }
    }

    free(args);
    for(int i = 0; i < NUM_THREADS; ++i)
        pthread_join(threads[i], NULL);

    for (auto entry: topTen)
        {
        std::cout << entry.rank << ' ' << entry.url << std::endl;
        }
}