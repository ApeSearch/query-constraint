#include "../include/ISR.h"

ISR::ISR() {}

ISRWord::ISRWord() : ISR() {}

ISRWord::ISRWord(PostingList * _posts) : ISR(), posts(_posts){}

ISREndDoc::ISREndDoc() : ISRWord() {}

ISREndDoc::ISREndDoc(PostingList* _posts) : ISRWord(_posts) {}

// Return the number of documents that contain this word
unsigned ISRWord::GetDocumentCount( ) { return posts->numOfDocs; }

unsigned ISRWord::GetNumberOfOccurrences( ) { return posts->numberOfPosts; }

Post *ISRWord::GetCurrentPost( ) {

    return nullptr;
}

Post * ISRWord::Next( ) {
    return GetCurrentPost();
}

Post * ISRWord::NextDocument( ) {
    return nullptr;
}

Post * ISRWord::Seek( Location target ) {
    return nullptr;
}

Location ISRWord::GetStartLocation( ) {}

Location ISRWord::GetEndLocation( ) {}


unsigned ISREndDoc::GetDocumentLength( ) {}
unsigned ISREndDoc::GetTitleLength( ) {}
unsigned ISREndDoc::GetUrlLength( ) {}

ISROr::ISROr() : ISR(), terms(nullptr), numTerms(0) {} 

ISRAnd::ISRAnd() : ISR(), terms(nullptr), numTerms(0) {}

ISRPhrase::ISRPhrase() : ISR(), terms(nullptr), numTerms(0) {}

ISRContainer::ISRContainer() : ISR(), contained(nullptr), excluded(nullptr), endDoc(nullptr), 
    countContained(0), countExcluded(0) {} 