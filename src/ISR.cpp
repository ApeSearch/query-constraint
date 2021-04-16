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



ISRAnd::ISRAnd() {}

ISRPhrase::ISRPhrase() {}

ISRContainer::ISRContainer() {} 

/*  

ISROr

*/

ISROr::ISROr() {} 
ISROr::ISROr(IndexHT *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), nearestTerm(0) {} 

Post * ISROr::Seek( Location target, Location endDocLoc )
    {
    // Seek all the ISRs to the first occurrence beginning at
    // the target location. Return null if there is no match.
    // The document is the document containing the nearest term.
    if (!numTerms) return nullptr;
    Location docStartLoc = target;
    Location docEndLoc = endDocLoc;
    // Max found location = end of doc location
    Location minLocation = docEndLoc;
    for (int i = 0; i < numTerms; ++i) 
        {
        if (!terms[i]) continue;
        Post * foundPost = terms[i]->Seek(target, endDocLoc);
        if (foundPost) 
            {
            // Ensure that the found location is within the document boundaries
            // must be >= docStartLoc to ensure that the first word in the index is found
            if (foundPost->loc < minLocation && foundPost->loc >= docStartLoc)
                {
                post = foundPost;
                nearestTerm = i;
                minLocation = foundPost->loc;
                nearestStartLocation = minLocation;
                }
            }
        }
        
    return (minLocation < docEndLoc) ? post : nullptr;
    }

Post * ISROr::Next( Location endDocLoc )
    {
    // Do a next on the nearest term
    Post* nearestNextPost = terms[nearestTerm]->Next(endDocLoc);
    unsigned currentNearest = nearestTerm;
    Location thisLoc = terms[nearestTerm]->GetStartLocation();
    
    Location minLocation = endDocLoc;
    // Return the new nearest match.
    for (int i = 0; i < numTerms; ++i) {
        if (!terms[i]) continue;
        Location currentLoc = terms[i]->GetStartLocation();
        if (currentLoc < minLocation ) {
            nearestTerm = i;
            nearestStartLocation = currentLoc;
            minLocation = currentLoc;
        }  
    }
    Post *newNearest = terms[nearestTerm]->Seek(minLocation, endDocLoc);
    if (!newNearest) nearestStartLocation = endDocLoc;
    else if (newNearest->loc > endDocLoc) {
        nearestStartLocation = endDocLoc;
        return nullptr;
    }
    
    return newNearest;
    }

// end ISROr
/*

ISRAnd

*/

ISRAnd::ISRAnd(IndexHT *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), nearestTerm(0), farthestTerm(0) {}

Post * ISRAnd::Seek( Location target, Location endDocLoc ) 
    {
    // 1. Seek all the ISRs to the first occurrence beginning at
    //    the target location.

    // 2. Move the document end ISR to just past the furthest
    //    word, then calculate the document begin location.

    // 3. Seek all the other terms to past the document begin.

    // 4. If any term is past the document end, return to
    //    step 2.

    // 5. If any ISR reaches the end, there is no match.

    nearestStartLocation = endDocLoc;
    nearestEndLocation = target;
    for (int i = 0; i < numTerms; ++i) {\
        // If one of the terms doesn't have a posting list, this document isn't a match
        if (terms[i] == nullptr) return nullptr;

        Post* foundPost = terms[i]->Seek(target, endDocLoc);
        if (!foundPost || foundPost->loc > endDocLoc) return nullptr;
        if (foundPost->loc >= nearestEndLocation)
            {
            nearestEndLocation = foundPost->loc;
            farthestTerm = i;
            }
        if (foundPost->loc <= nearestStartLocation)
            {
            post = foundPost;
            nearestStartLocation = foundPost->loc;
            nearestTerm = i;
            }
    }

    return post;
    }

Post * ISRAnd::Next( Location endDocLoc )
    {
    Post* nearestNextPost = terms[nearestTerm]->Next(endDocLoc);
    unsigned currentNearest = nearestTerm;
    Location thisLoc = terms[nearestTerm]->GetStartLocation();
    
    Location minLocation = endDocLoc;
    // Return the new nearest match.
    for (int i = 0; i < numTerms; ++i) {
        if (!terms[i]) continue;
        Location currentLoc = terms[i]->GetStartLocation();
        if (currentLoc < minLocation ) {
            nearestTerm = i;
            nearestStartLocation = currentLoc;
            minLocation = currentLoc;
        }  
    }
    Post *newNearest = terms[nearestTerm]->Seek(minLocation, endDocLoc);
    if (!newNearest) nearestStartLocation = endDocLoc;
    else if (newNearest->loc > endDocLoc) {
        nearestStartLocation = endDocLoc;
        return nullptr;
    }
    
    return newNearest;
    }

ISRPhrase::ISRPhrase(IndexHT *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), nearestTerm(0), farthestTerm(0) {}

ISRContainer::ISRContainer(IndexHT *_indexPtr) : ISR(_indexPtr), contained(nullptr), excluded(nullptr), 
    countContained(0), countExcluded(0) {} 