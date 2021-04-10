#include "../include/ISR.h"

ISR::ISR() {}

ISR::ISR(IndexHT *_indexPtr) : indexPtr(_indexPtr) {}

ISRWord::ISRWord() : ISR() {}

ISRWord::ISRWord(PostingList * _posts, IndexHT *_indexPtr) : ISR(_indexPtr), posts(_posts), postIndex(0) {}

ISREndDoc::ISREndDoc() : ISRWord() {}

ISREndDoc::ISREndDoc(PostingList* _posts, IndexHT *_indexPtr) : ISRWord(_posts, _indexPtr) {}

// Return the number of documents that contain this word
unsigned ISRWord::GetDocumentCount( ) { }

unsigned ISRWord::GetNumberOfOccurrences( ) { }

Post *ISRWord::GetCurrentPost( ) {
    return ;
}

Post * ISRWord::Next( ) {
    return GetCurrentPost();
}

Post * ISRWord::NextDocument( ) {
    return nullptr;
}

Post * ISRWord::Seek( Location target ) {
    return posts->Seek(target);
}

Location ISRWord::GetStartLocation( ) {}

Location ISRWord::GetEndLocation( ) {}


unsigned ISREndDoc::GetDocumentLength( ) {}
unsigned ISREndDoc::GetTitleLength( ) {}
unsigned ISREndDoc::GetUrlLength( ) {}

ISROr::ISROr() {} 

ISRAnd::ISRAnd() {}

ISRPhrase::ISRPhrase() {}

ISRContainer::ISRContainer() {} 

ISROr::ISROr(IndexHT *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), DocumentEnd(indexPtr->getEndDocISR()) {} 

ISRAnd::ISRAnd(IndexHT *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), DocumentEnd(indexPtr->getEndDocISR()) {}

ISRPhrase::ISRPhrase(IndexHT *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0) {}

ISRContainer::ISRContainer(IndexHT *_indexPtr) : ISR(_indexPtr), contained(nullptr), excluded(nullptr), endDoc(nullptr), 
    countContained(0), countExcluded(0) {} 