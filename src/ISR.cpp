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