#include "../include/ISR.h"

ISR::ISR() {}

ISR::ISR(IndexHT *_indexPtr) : indexPtr(_indexPtr) {}

ISRWord::ISRWord() : ISR() {}

ISRWord::ISRWord(PostingList * _posts, IndexHT *_indexPtr, APESEARCH::string _word) : ISR(_indexPtr), posts(_posts), postIndex(0), startLocation(posts->posts[0]->loc),
                                                            endLocation(posts->posts[0]->loc), word(_word) {}

ISRWord::ISRWord(PostingList * _posts, IndexHT *_indexPtr, APESEARCH::string _word, Location _start) : ISRWord(_posts, _indexPtr, _word) {
    startLocation = _start;
}

ISREndDoc::ISREndDoc() : ISRWord() {}

ISREndDoc::ISREndDoc(PostingList* _posts, IndexHT *_indexPtr) : ISRWord(_posts, _indexPtr, "%", 0) {}

// Return the number of documents that contain this word
unsigned ISRWord::GetDocumentCount( ) { }

unsigned ISRWord::GetNumberOfOccurrences( ) { }

Post *ISRWord::GetCurrentPost( ) {
    return posts->Seek(startLocation);
}

Post * ISRWord::Next( Location endDocLoc ) {
    if (++postIndex < posts->posts.size()) 
        {
        Post * next = posts->posts[postIndex];
        startLocation = next->loc;
        return next;
        }
    startLocation = endDocLoc;
    return nullptr;
}

Post * ISREndDoc::Next( Location endDocLoc ) {
    startLocation = endLocation;
    if (++postIndex < posts->posts.size()) 
        {
        Post * next = posts->posts[postIndex];
        endLocation = next->loc;
        return next;
        }
    return nullptr;
}


Post * ISRWord::Seek( Location target, Location docEndLoc ) {
    Post *found = posts->Seek(target);
    if (found)
        startLocation = found->loc;
    return found;
}

Post * ISREndDoc::Seek( Location target, Location docEndLoc ) {
    Post *found = posts->Seek(target);
    if (found)
        startLocation = endLocation;
        endLocation = found->loc;
    return found;
}

Location ISRWord::GetStartLocation( ) {
    return startLocation;
}

Location ISRWord::GetEndLocation( ) {
    return endLocation;
}

Post* ISRWord::NextDocument( Location docStartLoc, Location docEndLoc ) {

    Post *seeked = Seek(docStartLoc, docEndLoc);
    
    return (seeked && (seeked->loc < docStartLoc)) ? seeked : nullptr;
}

unsigned ISREndDoc::GetDocumentLength( ) {}
unsigned ISREndDoc::GetTitleLength( ) {}
unsigned ISREndDoc::GetUrlLength( ) {}

ISROr::ISROr() {} 

ISRAnd::ISRAnd() {}

ISRPhrase::ISRPhrase() {}

ISRContainer::ISRContainer() {} 

ISROr::ISROr(IndexHT *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), nearestTerm(0) {} 

ISRAnd::ISRAnd(IndexHT *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), nearestTerm(0), farthestTerm(0) {}

ISRPhrase::ISRPhrase(IndexHT *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), nearestTerm(0), farthestTerm(0) {}

ISRContainer::ISRContainer(IndexHT *_indexPtr) : ISR(_indexPtr), contained(nullptr), excluded(nullptr), 
    countContained(0), countExcluded(0) {} 