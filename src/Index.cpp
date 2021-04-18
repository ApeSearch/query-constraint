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