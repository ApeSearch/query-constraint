#include "../include/ISR.h"
#include "../include/Index.h"


ISR::ISR() {}

ISR::ISR(const IndexBlob *_indexPtr) : indexPtr(_indexPtr) {}

ISRWord::ISRWord() : ISR() {}

ISRWord::ISRWord(ListIterator *  _posts, const IndexBlob *_indexPtr, APESEARCH::string _word) : ISR(_indexPtr), posts(_posts), postIndex(0), word(_word) 
    {
    startLocation = posts->prevLoc;
    endLocation = posts->prevLoc;
    }

ISREndDoc::ISREndDoc() : ISRWord() {}

ISREndDoc::ISREndDoc(ListIterator *  _posts, const IndexBlob *_indexPtr) : ISRWord(_posts, _indexPtr, "%") {
    startLocation = endLocation = posts->prevLoc;
}

// Return the number of documents that contain this word
unsigned ISRWord::GetDocumentCount( ) { }

unsigned ISRWord::GetNumberOfOccurrences( ) { }

Post *ISRWord::GetCurrentPost( ) {
    return posts->Seek(startLocation);
}

Post * ISRWord::Next( ISREndDoc* docEnd ) {
    // if (++postIndex < posts->posts.size()) 
    //     {
    //     Post * next = posts->posts[postIndex];
    //     startLocation = next->loc;
    //     return next;
    //     }
    return posts->Next();
}

Post * ISREndDoc::Next( ISREndDoc* docEnd ) {

    // startLocation = endLocation;
    // if (++postIndex < posts->posts.size()) 
    //     {
    //     Post * next = posts->posts[postIndex];
    //     endLocation = next->loc;
    //     return next;
    //     }
    return posts->Next();
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
        startLocation = posts->prevLoc;
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

//never used
Post* ISRWord::NextDocument( ISREndDoc* docEnd ) {
    // Post *seeked = Seek( docEnd->GetEndLocation() + 1, docEnd );
    
    // return (seeked && (seeked->loc < docStartLoc)) ? seeked : nullptr;
}

unsigned ISREndDoc::GetTitleLength( ) {}



ISRAnd::ISRAnd() {}

ISRPhrase::ISRPhrase() {}

ISRContainer::ISRContainer() {} 

/*  

ISROr

*/

ISROr::ISROr() {} 
ISROr::ISROr(const IndexBlob *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), nearestTerm(0) {} 

Post * ISROr::Seek( Location target, ISREndDoc* docEnd )
    {
    // foundPosts = {};
    Location minLoc = (Location)-1;
    Post *nearestPost = nullptr;
    for (int i = 0; i < numTerms; ++i) {
        if (!terms[i]) continue;
        Post * foundPost = terms[i]->Seek(target, docEnd);
        if (foundPost) {
            // foundPosts.push_back(i);
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
    
    Location minLocation = (Location) - 1;
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
    Post *newNearest = terms[nearestTerm]->Seek(minLocation, docEnd);
    
    return newNearest;
    }

// end ISROr
/*

ISRAnd

*/

ISRAnd::ISRAnd(const IndexBlob *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), nearestTerm(0), farthestTerm(0) {}

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
        // foundPosts = {};
        // 2. Move the document end ISR to just past the furthest
        //    word, then calculate the document begin location.
        Post * enddocPost = docEnd->Seek(nearestEndLocation, docEnd);

        Location documentBegin = docEnd->posts->prevLoc;

        // 3. Seek all the other terms to past the document begin.
        bool found = true;
        for (int i = 0; i < numTerms; ++i) 
            {
            if (!terms[i]) continue;

            Post* foundPost = terms[i]->Seek(documentBegin, docEnd);
            if (foundPost) 
                {
                // foundPosts.push_back(i);
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
                if (foundPost->loc > docEnd->GetEndLocation()) 
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
    return Seek( nearestStartLocation + 1, docEnd );
    }

ISRPhrase::ISRPhrase(const IndexBlob *_indexPtr) : ISR(_indexPtr), terms(nullptr), numTerms(0), nearestTerm(0), farthestTerm(0) {}

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
        Post * farthestPost = nullptr;
        nearestStartLocation = (Location) - 1;
        while (true)
        {   
            // foundPosts = {};
            bool found = true;
            for (int i = 0; i < numTerms; ++i) {
                if (!terms[i]) return nullptr;
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
                    // foundPosts.push_back(i);
                    post = terms[i]->Seek(expectedLoc, docEnd);
                    if (!post)
                        return nullptr;

                    nearestStartLocation = (post->loc <= nearestStartLocation) ? terms[(nearestTerm = i)]->GetStartLocation() : nearestStartLocation;
                    nearestEndLocation = (post->loc >= nearestEndLocation) ? terms[(farthestTerm = i)]->GetStartLocation() : nearestEndLocation;

                    // 3. If any term is past the desired location, return
                    //    to step 2.
                    if (post->loc > expectedLoc) 
                        {
                        found = false;
                        break;
                        }
                    }
                    // nearestEndLocation = (post->loc >= nearestEndLocation) ? terms[(farthestTerm = i)]->GetStartLocation() : nearestEndLocation;
                    
                    if (i == nearestTerm) {
                        nearestPost = post;
                    } 
                    else if (i == farthestTerm) {
                        farthestPost = post;
                    }
                        
                    
            }
            if (found)
                {
                if (farthestTerm < nearestTerm)
                    {
                    return farthestPost;
                    }
                return nearestPost;
                }
        }
        return nullptr;


        // 4. If any ISR reaches the end, there is no match.
    }

Post * ISRPhrase::Next( ISREndDoc* docEnd )
    {
    return Seek( nearestStartLocation + 1, docEnd );
    }

ISRContainer::ISRContainer(const IndexBlob *_indexPtr) : ISR(_indexPtr), contained(nullptr), excluded(nullptr), 
    countContained(0), countExcluded(0) {} 

Post * ISRContainer::Seek(Location target, ISREndDoc* docEnd)
    {
    // 1. Seek all the ISRs to the first occurrence beginning at
    //    the target location.
    nearestStartLocation = (Location) - 1;
    nearestEndLocation = target;
    post = nullptr;
    for (int i = 0; i < countContained; ++i) {\
        if (!contained[i]) return nullptr;

        Post* foundPost = contained[i]->Seek(target, docEnd);
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
        // foundPosts = {};
        // 2. Move the document end ISR to just past the furthest
        //    word, then calculate the document begin location.
        Post * enddocPost = docEnd->Seek(nearestEndLocation, docEnd);

        Location documentBegin = docEnd->posts->prevLoc;

        // 3. Seek all the other terms to past the document begin.
        bool found = true;
        for (int i = 0; i < countContained; ++i) 
            {
            if (!contained[i]) continue;

            Post* foundPost = contained[i]->Seek(documentBegin, docEnd);
            if (foundPost) 
                {
                // foundPosts.push_back(i);
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
                if (foundPost->loc > docEnd->GetEndLocation()) 
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
                {
                // if (excluded) {
                //     excluded->Seek(target, endDocLoc);
                //     Location loceroni = excluded->GetStartLocation();
                //     return (loceroni < endDocLoc) ? post : nullptr;
                // }
                return post;
                }
        }
    
    return nullptr;
    }