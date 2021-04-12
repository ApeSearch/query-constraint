
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
        virtual Post *NextDocument( ) = 0;
        virtual Post *Seek( Location target ) = 0;
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
        ~ISRWord() {delete posts;}

        unsigned GetDocumentCount( );
        unsigned GetNumberOfOccurrences( );
        virtual Post *GetCurrentPost( );

        virtual Post *Next( ) override;
        Post *Seek( Location target ) override;

        Location GetStartLocation( ) override;
        Location GetEndLocation( ) override;

        Post *NextDocument( ) override;

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

    };

class ISROr : public ISR
    {
        public:
            ISROr();
            ISROr(IndexHT *_indexPtr);
            ~ISROr() {
                for (auto i = 0; i < numTerms; ++i) {
                    delete terms[i];
                }
                delete DocumentEnd;
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

            Post *Seek( Location target ) override
                {
                // Seek all the ISRs to the first occurrence beginning at
                // the target location. Return null if there is no match.
                // The document is the document containing the nearest term.
                if (!numTerms) return nullptr;

                Location docStartLoc = DocumentEnd->GetStartLocation();
                Location docEndLoc = DocumentEnd->GetEndLocation();

                // Max found location = end of doc location
                Location minLocation = docEndLoc;
                for (int i = 0; i < numTerms; ++i) 
                    {
                    if (terms[i]->Seek(target)) 
                        {
                        Location currentLoc = terms[i]->GetStartLocation();

                        // Ensure that the found location is within the document boundaries
                        // must be >= docStartLoc to ensure that the first word in the index is found
                        if (currentLoc < minLocation && currentLoc >= docStartLoc)
                            {
                            nearestTerm = i;
                            minLocation = currentLoc;
                            }
                        }
                    }
                    
                return (minLocation < docEndLoc) ? DocumentEnd->GetCurrentPost() : nullptr;
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
                return terms[nearestTerm]->Seek(terms[nearestTerm]->GetStartLocation());
                }

            Post *NextDocument( ) override
                {
                // Seek all the ISRs to the first occurrence just past
                // the end of this document.
                Post* seeked = Seek( DocumentEnd->GetStartLocation( ) );
                DocumentEnd->Next();

                return seeked;
                }

            void initNearest() {
                for (int i = 0; i < numTerms; ++i) {
                    if (terms[i]) {
                        Location loc = terms[i]->GetStartLocation();
                        if (loc < nearestStartLocation) {
                            nearestTerm = i;
                            nearestStartLocation = terms[i]->GetStartLocation();
                        }
                    }
                }
            }
        
        private:
            unsigned nearestTerm;
            Location nearestStartLocation, nearestEndLocation;
            ISREndDoc *DocumentEnd;
    };

class ISRAnd : public ISR 
    {
        public:
            ISRAnd();
            ISRAnd(IndexHT *_indexPtr);
            ~ISRAnd() {
                for (auto i = 0; i < numTerms; ++i) {
                    delete terms[i];
                }
            }

            ISR **terms;
            unsigned numTerms;

            Post *Seek( Location target ) override
                {
                    // 1. Seek all the ISRs to the first occurrence beginning at
                    //    the target location.
                    for (int i = 0; i < numTerms; ++i) 
                        terms[i]->Seek(target);

                    // 2. Move the document end ISR to just past the furthest
                    //    word, then calculate the document begin location.
                    DocumentEnd->Seek( terms[farthestTerm]->GetStartLocation() );

                    // 3. Seek all the other terms to past the document begin.

                    // 4. If any term is past the document end, return to
                    //    step 2.

                    // 5. If any ISR reaches the end, there is no match.
                }

            Post * Next() override
                {
                return Seek( nearestStartLocation + 1 );
                }

            Post *NextDocument( ) override
                {
                // Seek all the ISRs to the first occurrence just past
                // the end of this document.
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
            unsigned nearestTerm, farthestTerm;
            Location nearestStartLocation, nearestEndLocation;

            ISREndDoc *DocumentEnd;
    };

class ISRPhrase : public ISR
    {
    public:
        ISRPhrase();
        ISRPhrase(IndexHT *_indexPtr);

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
        Post *NextDocument( ) override
            {
            // Seek all the ISRs to the first occurrence just past
            // the end of this document.
            Post* seeked = Seek( DocumentEnd->GetStartLocation( ) );
            DocumentEnd->Next();

            return seeked;
            }

        Post *Seek( Location target ) override
            {
                // 1. Seek all ISRs to the first occurrence beginning at
                //    the target location.
                Location docStartLoc = DocumentEnd->GetStartLocation();
                Location docEndLoc = DocumentEnd->GetEndLocation();

                nearestStartLocation = DocumentEnd->GetEndLocation();
                nearestEndLocation = target;
                for (int i = 0; i < numTerms; ++i) {\
                    // If one of the terms doesn't have a posting list, it won't be found
                    if (terms[i] == nullptr) return nullptr;

                    Post* post = terms[i]->Seek(target);
                    if (terms[i]->GetStartLocation() >= docStartLoc) 
                        {
                        if (post->loc > nearestEndLocation)
                            {
                            nearestEndLocation = post->loc;
                            farthestTerm = i;
                            }
                        else if (post->loc < nearestStartLocation)
                            {
                            nearestStartLocation = post->loc;
                            nearestTerm;
                            }
                        }
                }

                // 2. Pick the furthest term and attempt to seek all
                //    the other terms to the first location beginning
                //    where they should appear relative to the furthest
                //    term.
                for (int i = 0; i < numTerms; ++i) {
                    Post *post;
                    if (i < farthestTerm) {
                        post = terms[i]->Seek(nearestEndLocation - farthestTerm - i);
                    } else if (i > farthestTerm) {
                        post = terms[i]->Seek(nearestEndLocation + farthestTerm + i);
                    }
                    
                }



                if (nearestEndLocation > docEndLoc) 
                    return nullptr;


                // 3. If any term is past the desired location, return
                //    to step 2.

                // 4. If any ISR reaches the end, there is no match.
                return nullptr;
            }

        Post * Next() override
            {
            return Seek( nearestStartLocation + 1 );
            }
    private: 
        ISREndDoc *DocumentEnd;
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
        ISREndDoc *endDoc;
        unsigned countContained, countExcluded;
        // Location Next( );
        
        Post *Seek( Location target ) override
            {
            // 1. Seek all the included ISRs to the first occurrence beginning at
            //    the target location.
            
            // 2. Move the document end ISR to just past the furthest
            //    contained ISR, then calculate the document begin location.

            // 3. Seek all the other contained terms to past the document begin.

            // 4. If any contained term is past the document end, return to
            //    step 2.

            // 5. If any ISR reaches the end, there is no match.

            // 6. Seek all the excluded ISRs to the first occurrence beginning at
            //    the document begin location.// 7. If any excluded ISR falls within the document, reset the
            //    target to one past the end of the document and return to
            //    step 1.
            }

        Post *Next( ) override
            {
            Seek( contained[ nearestContained ]->GetStartLocation( ) + 1 );
            }

    private:
        unsigned nearestTerm, farthestTerm;
        unsigned nearestContained;
        Location nearestStartLocation, nearestEndLocation;
    };

#endif