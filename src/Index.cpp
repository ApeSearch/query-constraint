#include "../include/Index.h"

ISRWord *IndexBlob::getWordISR ( const IndexBlob* blob, APESEARCH::string word ) const
    {
        const SerializedPostingList* entry = Find(word);
        return entry ? new ISRWord(new ListIterator(entry), blob, word) : nullptr;
    }
ISREndDoc *IndexBlob::getEndDocISR ( const IndexBlob* blob ) const
    {
        const SerializedPostingList* entry = Find(APESEARCH::string("%"));
        return new ISREndDoc(new ListIterator(entry), blob);
    }

void Index::searchIndexChunks(APESEARCH::string queryLine) {
    for (int i = 0; i < chunkFileNames.size(); ++i) {
        // Build the parse tree (done for every index chunk because the parse tree is deleted on Compile())
        APESEARCH::unique_ptr<query::Tuple> parseTree = buildParseTree(queryLine); 
        IndexFile chunkFile (chunkFileNames[i].cstr());
        const IndexBlob* chunk = chunkFile.Blob();

        APESEARCH::unique_ptr<ISR> compiledTree = APESEARCH::unique_ptr<ISR>(parseTree->Compile(chunk));
        APESEARCH::unique_ptr<ISREndDoc> docEnd = APESEARCH::unique_ptr<ISREndDoc>(chunk->getEndDocISR(chunk));

        Post *post = compiledTree->NextDocument(docEnd.get());

        cout << chunkFileNames[i] << " ";

        while (post) {
            docEnd->Seek(post->loc, docEnd.get());
            cout << post->loc << " ";

            // Do this in the ranker? 
            compiledTree->Seek(docEnd->GetStartLocation(), docEnd.get());

            // Def do this in ranker
            APESEARCH::vector<ISR *> flattened;
            compiledTree->Flatten(flattened);
            

            post = compiledTree->NextDocument(docEnd.get());
        }

        cout << endl;

        // solve constraint on index chunk using ISR tree
        // for each matching document
            // isrTree.Seek(beginning of document) 
            // rank = rankerClass.getRank(isrTree)
            // struct RankStruct = {double rank, string documentURL};
            // top10<RankStruct>.insert(rank, matching document)
    }
}