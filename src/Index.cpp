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

        char * beg = (char *) this + VectorStart;
        char * end = beg;
        
        while(end < (char *) this + BlobSize){
            while(*end != '\0') end++;

            if(end >= (char *) this + BlobSize)
                return vecUrls;

            else
            {
                auto url = APESEARCH::string(beg);

                vecUrls.emplace_back(url);

                end++;
                beg = end;
            }

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
    while (true) {
        pthread_mutex_lock(&queueLock);
        while(index->threadQueue.empty() && index->filesToPush)
            pthread_cond_wait(&notEmpty, &queueLock);

        if (!index->filesToPush && index->threadQueue.empty())
            {
                pthread_mutex_unlock(&queueLock);
                break;
            }
            

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
}


void Index::searchIndexChunks(const char * queryLine) {

    topTen = APESEARCH::vector<RankedEntry>(10);

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

        pthread_cond_broadcast(&notEmpty);
        pthread_mutex_unlock(&queueLock);
    }

    pthread_mutex_lock(&queueLock);
    filesToPush = false;
    pthread_mutex_unlock(&queueLock);
    
    for(int i = 0; i < NUM_THREADS; ++i)
        pthread_join(threads[i], NULL);

    free(args);

    for (auto entry: topTen)
        {
        std::cout << entry.rank << ' ' << entry.url << std::endl;
        }
}