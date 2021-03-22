#include "../include/ISR.h"

ISR::ISR() {}

ISRWord::ISRWord() : ISR() {}
ISRWord::ISRWord(std::string _word) : ISR(), word(_word) {}
unsigned ISRWord::GetDocumentCount( ) {}
unsigned ISRWord::GetNumberOfOccurrences( ) {}
Post *ISRWord::GetCurrentPost( ) {}

ISREndDoc::ISREndDoc() {}

unsigned ISREndDoc::GetDocumentLength( ) {}
unsigned ISREndDoc::GetTitleLength( ) {}
unsigned ISREndDoc::GetUrlLength( ) {}

ISROr::ISROr() : ISR(), terms(nullptr), numTerms(0) {} 

ISRAnd::ISRAnd() : ISR(), terms(nullptr), numTerms(0) {}

ISRPhrase::ISRPhrase() : ISR(), terms(nullptr), numTerms(0) {}

ISRContainer::ISRContainer() : ISR(), contained(nullptr), excluded(nullptr), endDoc(nullptr), 
    countContained(0), countExcluded(0) {} 