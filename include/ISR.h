
#pragma once

#ifndef _ISR_H
#define _ISR_H

#include "assert.h"
#include "HashTable.h"
#include <unistd.h>
#include <utility>
#include <iostream>
#include "../libraries/AS/include/AS/string.h"
#include "IndexHT.h"

class ISR //fix inheritance to be logical, remove duplicate code and member variables
    {

    public:
        ISR();
        ISR(IndexHT *_indexPtr);
        virtual ~ISR() {
        }

        // Store information the index provides i.e. the posting list, the location 
        // ( so as to provide information about how to search for the next location or a post assoicated with a location)
        virtual Post *Next( ) = 0;
        virtual Post *NextDocument( Location docStartLoc, Location docEndLoc  ) = 0;
        virtual Post *Seek( Location target, Location endDocLoc ) = 0;
        virtual Location GetStartLocation( ) = 0;
        virtual Location GetEndLocation( ) = 0;

        IndexHT *indexPtr;
    };

class ISRWord : public ISR
    {
    public:
        ISRWord();
        ISRWord(PostingList * _posts, IndexHT *indexPtr);
        ISRWord(PostingList * _posts, IndexHT *_indexPtr, Location _start);
        ~ISRWord() {

        }

        unsigned GetDocumentCount( );
        unsigned GetNumberOfOccurrences( );
        virtual Post *GetCurrentPost( );

        virtual Post *Next( ) override;
        virtual Post *Seek( Location target, Location endDocLoc ) override;

        Location GetStartLocation( ) override;
        Location GetEndLocation( ) override;

        Post *NextDocument( Location docStartLoc, Location docEndLoc  ) override;

    // private:
        PostingList* posts;
        Location startLocation, endLocation;
        unsigned postIndex;
    };

class ISREndDoc : public ISRWord
    {
    public:
        ISREndDoc();
        ISREndDoc(PostingList* _posts, IndexHT *indexPtr);

        unsigned GetDocumentLength( );
        unsigned GetTitleLength( );
        unsigned GetUrlLength( );

        Post *Next( ) override;
        Post *Seek( Location target, Location endDocLoc ) override;

    };

class ISROr : public ISR
    {
        public:
            ISROr();
            ISROr(IndexHT *_indexPtr);
            ~ISROr() {
                for (int i = 0; i < numTerms; ++i) {
                    delete terms[i];
                }
                delete [] terms;
            }

            ISR **terms;
            unsigned numTerms;

            Location GetStartLocation() override
                {
                    return nearestStartLocation;
                }

            Location GetEndLocation() override
                {
                    return nearestEndLocation;
                }

            Post *Seek( Location target, Location endDocLoc ) override
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
                            }
                        }
                    }
                    
                return (minLocation < docEndLoc) ? post : nullptr;
                }

            Post *Next( ) override
                {
                // Do a next on the nearest term
                terms[nearestTerm]->Next();
                // Return the new nearest match.
                nearestTerm = 0;
                for (int i = 0; i < numTerms; ++i) {
                    Location currentLoc = terms[i]->GetStartLocation();
                    Location minLoc = terms[nearestTerm]->GetStartLocation();
                    if (currentLoc < minLoc ) {
                        nearestTerm = i;
                        nearestStartLocation = currentLoc;
                    }
                        
                }
                return nullptr;
                // return terms[nearestTerm]->Seek(terms[nearestTerm]->GetStartLocation(), );
                }

            Post *NextDocument( Location docStartLoc, Location docEndLoc  ) override
                {
                // Seek all the ISRs to the first occurrence just past
                // the end of this document.
                return Seek( docStartLoc, docEndLoc );
                }
        
        private:
            Post * post;

            unsigned nearestTerm;
            Location nearestStartLocation, nearestEndLocation;
    };

class ISRAnd : public ISR 
    {
        public:
            ISRAnd();
            ISRAnd(IndexHT *_indexPtr);
            ~ISRAnd() {
                for (int i = 0; i < numTerms; ++i) {
                    delete terms[i];
                }
                delete [] terms;
            }

            ISR **terms;
            unsigned numTerms;

            Post *Seek( Location target, Location endDocLoc ) override
                {
                // 1. Seek all the ISRs to the first occurrence beginning at
                //    the target location.
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

                // 2. Move the document end ISR to just past the furthest
                //    word, then calculate the document begin location.

                // 3. Seek all the other terms to past the document begin.

                // 4. If any term is past the document end, return to
                //    step 2.

                // 5. If any ISR reaches the end, there is no match.
                }

            Post * Next() override
                {
                // return Seek( nearestStartLocation + 1 );
                }

            Post *NextDocument( Location docStartLoc, Location docEndLoc ) override
                {
                // Seek all the ISRs to the first occurrence just past
                // the end of this document.
                return Seek( docStartLoc, docEndLoc );
                }

            Location GetStartLocation() override
                {
                return nearestStartLocation;
                }

            Location GetEndLocation() override
                {
                return nearestEndLocation;
                }

        private: 
            Post *post;

            unsigned nearestTerm, farthestTerm;
            Location nearestStartLocation, nearestEndLocation;
    };

class ISRPhrase : public ISR
    {
    public:
        ISRPhrase();
        ISRPhrase(IndexHT *_indexPtr);
        ~ISRPhrase() {
            for (int i = 0; i < numTerms; ++i) {
                delete terms[i];
            }
            delete [] terms;
        }

        ISR **terms;
        unsigned numTerms;

        Location GetStartLocation() override
            {
                return nearestStartLocation;
            }

        Location GetEndLocation() override
            {
                return nearestEndLocation;
            }
        Post *NextDocument( Location docStartLoc, Location docEndLoc  ) override
            {
            // Seek all the ISRs to the first occurrence just past
            // the end of this document.
            Post* seeked = Seek( docStartLoc, docEndLoc );
            // DocumentEnd->Next();

            return Seek( docStartLoc, docEndLoc );
            }

        Post *Seek( Location target, Location endDocLoc ) override
            {
                // 1. Seek all ISRs to the first occurrence beginning at
                //    the target location.
                nearestStartLocation = endDocLoc;
                nearestEndLocation = target;
                for (int i = 0; i < numTerms; ++i) {\
                    // If one of the terms doesn't have a posting list, it won't be found
                    if (terms[i] == nullptr) return nullptr;

                    Post* foundPost = terms[i]->Seek(target, endDocLoc);
                    if (!foundPost) return nullptr;
                    if (terms[i]->GetStartLocation() >= target) 
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
                        }
                }

                if (numTerms == 1 && nearestStartLocation < endDocLoc) {
                    return post;
                }

                // 2. Pick the furthest term and attempt to seek all
                //    the other terms to the first location beginning
                //    where they should appear relative to the furthest
                //    term.
                while (true)
                {
                    bool found = true;
                    for (int i = 0; i < numTerms; ++i) {
                        Location expectedLoc;
                        if (i < farthestTerm) {
                            expectedLoc = nearestEndLocation - farthestTerm + i;
                        } else if (i > farthestTerm) {
                            expectedLoc = nearestEndLocation + farthestTerm + i;
                        }
                        else continue;
                        if (found) {
                            post = terms[i]->Seek(expectedLoc, endDocLoc);
                            if (post == nullptr || post->loc > endDocLoc)
                                return nullptr;
                            if (post->loc != expectedLoc) {
                                nearestEndLocation = (post->loc > nearestEndLocation) ? terms[(farthestTerm = i)]->GetStartLocation() : nearestEndLocation;
                                nearestStartLocation = (post->loc < nearestStartLocation) ? terms[(nearestTerm = i)]->GetStartLocation() : nearestStartLocation;
                                found = false;
                                break;
                            }
                        }
                    }
                    if (found)
                        return post;
                }
                return nullptr;


                // 3. If any term is past the desired location, return
                //    to step 2.

                // 4. If any ISR reaches the end, there is no match.
            }

        Post * Next() override
            {
            // return Seek( nearestStartLocation + 1 );
            }
    private: 
        Post * post;

        unsigned nearestTerm, farthestTerm;
        Location nearestStartLocation, nearestEndLocation;
    };

class ISRContainer : public ISR
    {
    public:
        ISRContainer();
        ISRContainer(IndexHT *_indexPtr);

        ISR **contained; //List of ISRs to include
        ISR *excluded; //ISR to exclude

        unsigned countContained, countExcluded;
        // Location Next( );
        
        Post *Seek( Location target, Location endDocLoc ) override
            {
            // 1. Seek all the included ISRs to the first occurrence beginning at
            //    the target location.
            
            // 2. Move the document end ISR to just past the furthest
            //    contained ISR, then calculate the document begin location.

            // 3. Seek all the other contained terms to past the document begin.

            // 4. If any contained term is past the document end, return to
            //    step 2.

            // 5. If any ISR reaches the end, there is no match.
            nearestStartLocation = endDocLoc;
            nearestEndLocation = target;
            for (int i = 0; i < countContained; ++i) {\
                // If one of the terms doesn't have a posting list, this document isn't a match
                if (contained[i] == nullptr) return nullptr;

                Post* foundPost = contained[i]->Seek(target, endDocLoc);
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

            // 6. Seek all the excluded ISRs to the first occurrence beginning at
            //    the document begin location.
            if (excluded) {
                excluded->Seek(target, endDocLoc);
                Location loceroni = excluded->GetStartLocation();
                return (loceroni < endDocLoc) ? post : nullptr;
            }

            // 7. If any excluded ISR falls within the document, reset the
            //    target to one past the end of the document and return to
            //    step 1.
            return post;
            }

        Post *Next( ) override
            {
            // Seek( contained[ nearestContained ]->GetStartLocation( ) + 1 );
            }

        Location GetStartLocation() override
            {
                return nearestStartLocation;
            }

        Location GetEndLocation() override
            {
                return nearestEndLocation;
            }

        Post *NextDocument( Location docStartLoc, Location docEndLoc ) override
            {
            // Seek all the ISRs to the first occurrence just past
            // the end of this document.
            return Seek( docStartLoc, docEndLoc);;
            }

    private:
        Post * post;

        unsigned nearestTerm, farthestTerm;
        unsigned nearestContained;
        Location nearestStartLocation, nearestEndLocation;
    };

#endif