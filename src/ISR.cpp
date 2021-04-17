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

ISREndDoc::ISREndDoc(PostingList* _posts, IndexHT *_indexPtr) : ISRWord(_posts, _indexPtr, "%", 0) {
    startLocation = endLocation = 0;
}

// Return the number of documents that contain this word
unsigned ISRWord::GetDocumentCount( ) { }

unsigned ISRWord::GetNumberOfOccurrences( ) { }

Post *ISRWord::GetCurrentPost( ) {
    return posts->Seek(startLocation);
}

Post * ISRWord::Next( ISREndDoc* docEnd ) {
    if (++postIndex < posts->posts.size()) 
        {
        Post * next = posts->posts[postIndex];
        startLocation = next->loc;
        return next;
        }
    return nullptr;
}

Post * ISREndDoc::Next( ISREndDoc* docEnd ) {
    startLocation = endLocation;
    if (++postIndex < posts->posts.size()) 
        {
        Post * next = posts->posts[postIndex];
        endLocation = next->loc;
        return next;
        }
    return nullptr;
}


Post * ISRWord::Seek( Location target, ISREndDoc* docEnd ) {
    Post *found = posts->Seek(target);
    if (found)
        {
        startLocation = found->loc;
        }
        
    return found;
}

Post * ISREndDoc::Seek( Location target, ISREndDoc* docEnd ) {
    Post *found = posts->Seek(target);
    if (found)
        {
        startLocation = endLocation;
        endLocation = found->loc;
        }
    return found;
}

Location ISRWord::GetStartLocation( ) {
    return startLocation;
}

Location ISRWord::GetEndLocation( ) {
    return endLocation;
}

Post* ISRWord::NextDocument( ISREndDoc* docEnd ) {
    // Post *seeked = Seek( docEnd->GetEndLocation() + 1, docEnd );
    
    // return (seeked && (seeked->loc < docStartLoc)) ? seeked : nullptr;
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

Post * ISROr::Seek( Location target, ISREndDoc* docEnd )
    {
    Location minLoc = (Location)-1;
    Post *nearestPost = nullptr;
    for (int i = 0; i < numTerms; ++i) {
        if (!terms[i]) continue;
        Post * foundPost = terms[i]->Seek(target, docEnd);
        if (foundPost) {
            if (foundPost->loc < minLoc) {
                nearestPost = foundPost;
                nearestTerm = i;
                minLoc = foundPost->loc;
            }
        }
    }
    return nearestPost;
    }

Post * ISROr::Next( ISREndDoc* docEnd )
    {
    // Do a next on the nearest term
    Post* nearestNextPost = terms[nearestTerm]->Next(docEnd);
    // unsigned currentNearest = nearestTerm;
    // Location thisLoc = terms[nearestTerm]->GetStartLocation();
    
    // Location minLocation = endDocLoc;
    // // Return the new nearest match.
    // for (int i = 0; i < numTerms; ++i) {
    //     if (!terms[i]) continue;
    //     Location currentLoc = terms[i]->GetStartLocation();
    //     if (currentLoc < minLocation ) {
    //         nearestTerm = i;
    //         nearestStartLocation = currentLoc;
    //         minLocation = currentLoc;
    //     }  
    // }
    // Post *newNearest = terms[nearestTerm]->Seek(minLocation, endDocLoc);
    // if (!newNearest) nearestStartLocation = endDocLoc;
    // else if (newNearest->loc > endDocLoc) {
    //     nearestStartLocation = endDocLoc;
    //     return nullptr;
    // }
    
    // return newNearest;
    }

// end ISROr
/*

ISRAnd

*/

ISRAnd::ISRAnd(IndexHT *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), nearestTerm(0), farthestTerm(0) {}

Post * ISRAnd::Seek( Location target, ISREndDoc* docEnd ) 
    {
    // 1. Seek all the ISRs to the first occurrence beginning at
    //    the target location.
    nearestStartLocation = (Location) - 1;
    nearestEndLocation = target;
    post = nullptr;
    for (int i = 0; i < numTerms; ++i) {\
        if (!terms[i]) return nullptr;

        Post* foundPost = terms[i]->Seek(target, docEnd);
        if (foundPost) {
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
    }

    post = nullptr;
    nearestStartLocation = (Location) - 1;
    while (true) 
        {
        // 2. Move the document end ISR to just past the furthest
        //    word, then calculate the document begin location.
        Post * enddocPost = docEnd->Seek(nearestEndLocation, docEnd);

        if (!enddocPost) return nullptr;
        Location documentBegin = 0;
        Location next = docEnd->Seek(0, docEnd)->loc;
        while (next != enddocPost->loc) 
            {
            documentBegin = next;
            enddocPost = docEnd->Seek(0, docEnd);
            }
        docEnd->Next(docEnd);
        // 3. Seek all the other terms to past the document begin.
        bool found = true;
        for (int i = 0; i < numTerms; ++i) 
            {
            if (!terms[i]) continue;

            Post* foundPost = terms[i]->Seek(documentBegin, docEnd);
            if (foundPost) 
                {
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

                // 4. If any term is past the document end, return to
                //    step 2.
                if (foundPost->loc > docEnd->GetEndLocation() && (docEnd->GetStartLocation() != docEnd->GetEndLocation())) 
                    {
                    found = false;
                    break;
                    }
                } 
            else 
                {
                // 5. If any ISR reaches the end, there is no match.
                return nullptr;
                }
            } 

            if (found)
                return post;    
        }
    

    

    return nullptr;
    }

Post * ISRAnd::Next( ISREndDoc* docEnd )
    {
    // Post* nearestNextPost = terms[nearestTerm]->Next(endDocLoc);
    // unsigned currentNearest = nearestTerm;
    // Location thisLoc = terms[nearestTerm]->GetStartLocation();
    
    // Location minLocation = endDocLoc;
    // // Return the new nearest match.
    // for (int i = 0; i < numTerms; ++i) {
    //     if (!terms[i]) continue;
    //     Location currentLoc = terms[i]->GetStartLocation();
    //     if (currentLoc < minLocation ) {
    //         nearestTerm = i;
    //         nearestStartLocation = currentLoc;
    //         minLocation = currentLoc;
    //     }  
    // }
    // Post *newNearest = terms[nearestTerm]->Seek(minLocation, endDocLoc);
    // if (!newNearest) nearestStartLocation = endDocLoc;
    // else if (newNearest->loc > endDocLoc) {
    //     nearestStartLocation = endDocLoc;
    //     return nullptr;
    // }
    
    // return newNearest;
    }

ISRPhrase::ISRPhrase(IndexHT *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), nearestTerm(0), farthestTerm(0) {}

Post *ISRPhrase::Seek( Location target, ISREndDoc* docEnd )
    {
        // 1. Seek all ISRs to the first occurrence beginning at
        //    the target location.
        post = nullptr;
        nearestStartLocation = (Location) - 1;
        nearestEndLocation = target;
        for (int i = 0; i < numTerms; ++i) {
            // If one of the terms doesn't have a posting list, it won't be found
            if (!terms[i]) continue;

            Post* foundPost = terms[i]->Seek(target, docEnd);
            if (foundPost)
                {
                if (terms[i]->GetStartLocation() >= target) 
                    {
                    if (foundPost->loc >= nearestEndLocation)
                        {
                        post = foundPost;
                        nearestEndLocation = foundPost->loc;
                        farthestTerm = i;
                        }
                    if (foundPost->loc <= nearestStartLocation)
                        {
                        nearestStartLocation = foundPost->loc;
                        nearestTerm = i;
                        }
                    }
                }
            else return nullptr;
        }

        if (numTerms == 1 && post) {
            return post;
        }

        // 2. Pick the furthest term and attempt to seek all
        //    the other terms to the first location beginning
        //    where they should appear relative to the furthest
        //    term.
        Post * nearestPost = nullptr;
        nearestStartLocation = (Location) - 1;
        while (true)
        {
            bool found = true;
            for (int i = 0; i < numTerms; ++i) {
                if (!terms[i]) continue;
                Location expectedLoc;
                if (i < farthestTerm) {
                    expectedLoc = nearestEndLocation - farthestTerm + i;
                } else if (i > farthestTerm) {
                    expectedLoc = nearestEndLocation + farthestTerm + i;
                }
                else {
                    expectedLoc = nearestEndLocation;
                }
                if (found) 
                    {
                    post = terms[i]->Seek(expectedLoc, docEnd);
                    if (!post)
                        return nullptr;

                    nearestEndLocation = (post->loc >= nearestEndLocation) ? terms[(farthestTerm = i)]->GetStartLocation() : nearestEndLocation;
                    nearestStartLocation = (post->loc <= nearestStartLocation) ? terms[(nearestTerm = i)]->GetStartLocation() : nearestStartLocation;

                    if (i == nearestTerm)
                        nearestPost = post;
                        
                    // 3. If any term is past the desired location, return
                    //    to step 2.
                    if (post->loc > expectedLoc) 
                        {
                       
                        found = false;
                        break;
                        }
                    }
            }
            if (found)
                return nearestPost;
        }
        return nullptr;


        // 4. If any ISR reaches the end, there is no match.
    }

ISRContainer::ISRContainer(IndexHT *_indexPtr) : ISR(_indexPtr), contained(nullptr), excluded(nullptr), 
    countContained(0), countExcluded(0) {} 