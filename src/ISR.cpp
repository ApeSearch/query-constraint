#include "../include/ISR.h"

ISR::ISR() {}

ISR::ISR(IndexHT *_indexPtr) : indexPtr(_indexPtr) {}

ISRWord::ISRWord() : ISR() {}

ISRWord::ISRWord(PostingList * _posts, IndexHT *_indexPtr) : ISR(_indexPtr), posts(_posts), postIndex(0), startLocation(posts->posts[0]->loc), endLocation(posts->posts[0]->loc) {}

ISRWord::ISRWord(PostingList * _posts, IndexHT *_indexPtr, Location _start) : ISRWord(_posts, _indexPtr) {
    startLocation = _start;
}

ISREndDoc::ISREndDoc() : ISRWord() {}

ISREndDoc::ISREndDoc(PostingList* _posts, IndexHT *_indexPtr) : ISRWord(_posts, _indexPtr, 0) {}

// Return the number of documents that contain this word
unsigned ISRWord::GetDocumentCount( ) { }

unsigned ISRWord::GetNumberOfOccurrences( ) { }

Post *ISRWord::GetCurrentPost( ) {
    return posts->Seek(startLocation);
}

Post * ISRWord::Next( ) {
    if (++postIndex < posts->posts.size()) 
        {
        Post * next = posts->posts[postIndex];
        startLocation = next->loc;
        return next;
        }
    return nullptr;
}

Post * ISREndDoc::Next( ) {
    startLocation = endLocation;
    if (++postIndex < posts->posts.size()) 
        {
        Post * next = posts->posts[postIndex];
        endLocation = next->loc;
        return next;
        }
    return nullptr;
}


Post * ISRWord::Seek( Location target ) {
    Post *found = posts->Seek(target);
    if (found)
        startLocation = found->loc;
    return found;
}

Location ISRWord::GetStartLocation( ) {
    return startLocation;
}

Location ISRWord::GetEndLocation( ) {
    return endLocation;
}

Post* ISRWord::NextDocument( ) {
    return nullptr;
}

unsigned ISREndDoc::GetDocumentLength( ) {}
unsigned ISREndDoc::GetTitleLength( ) {}
unsigned ISREndDoc::GetUrlLength( ) {}

ISROr::ISROr() {} 

ISRAnd::ISRAnd() {}

ISRPhrase::ISRPhrase() {}

ISRContainer::ISRContainer() {} 

ISROr::ISROr(IndexHT *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), nearestTerm(0), DocumentEnd(indexPtr->getEndDocISR()) {} 

ISRAnd::ISRAnd(IndexHT *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), nearestTerm(0), farthestTerm(0), DocumentEnd(indexPtr->getEndDocISR()) {}

ISRPhrase::ISRPhrase(IndexHT *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), nearestTerm(0), farthestTerm(0), DocumentEnd(indexPtr->getEndDocISR()) {}

ISRContainer::ISRContainer(IndexHT *_indexPtr) : ISR(_indexPtr), contained(nullptr), excluded(nullptr), endDoc(nullptr), 
    countContained(0), countExcluded(0) {} 