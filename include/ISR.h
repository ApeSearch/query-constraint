
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
        virtual Post *Next( ISREndDoc* docEnd ) = 0;
        virtual Post *NextDocument( ISREndDoc* docEnd ) = 0;
        virtual Post *Seek( Location target, ISREndDoc* docEnd ) = 0;
        virtual Location GetStartLocation( ) = 0;
        virtual Location GetEndLocation( ) = 0;

        virtual APESEARCH::string GetNearestWord() = 0;

        IndexHT *indexPtr;
    };

class ISRWord : public ISR
    {
    public:
        ISRWord();
        ISRWord(PostingList * _posts, IndexHT *indexPtr, APESEARCH::string word);
        ISRWord(PostingList * _posts, IndexHT *_indexPtr, APESEARCH::string _word, Location _start);
        ~ISRWord() {

        }

        unsigned GetDocumentCount( );
        unsigned GetNumberOfOccurrences( );
        virtual Post *GetCurrentPost( );

        virtual Post *Next( ISREndDoc* docEnd ) override;
        virtual Post *Seek( Location target, ISREndDoc* docEnd ) override;

        Location GetStartLocation( ) override;
        Location GetEndLocation( ) override;

        Post *NextDocument( ISREndDoc* docEnd ) override;

        APESEARCH::string GetNearestWord() override 
            {
            return word;
            }

        APESEARCH::string word;
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

        Post *Next( ISREndDoc* docEnd ) override;
        Post *Seek( Location target, ISREndDoc* docEnd ) override;

        APESEARCH::string GetNearestWord() override 
            {
            return word;
            }
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

            Post *Seek( Location target, ISREndDoc* docEnd ) override;

            Post *Next( ISREndDoc* docEnd ) override;

            Post *NextDocument( ISREndDoc* docEnd ) override
                {
                return Seek( (docEnd->GetEndLocation() == 0) ? 0 : docEnd->GetEndLocation() + 1, docEnd );
                }

            APESEARCH::string GetNearestWord() override
                { 
                return terms[nearestTerm]->GetNearestWord();
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

            Post *Seek( Location target, ISREndDoc* docEnd ) override;

            Post * Next( ISREndDoc* docEnd ) override;

            Post *NextDocument( ISREndDoc* docEnd ) override
                {
                return Seek( (docEnd->GetEndLocation() == 0) ? 0 : docEnd->GetEndLocation() + 1, docEnd );
                }

            Location GetStartLocation() override
                {
                return nearestStartLocation;
                }

            Location GetEndLocation() override
                {
                return nearestEndLocation;
                }

            APESEARCH::string GetNearestWord() override 
                {
                return terms[nearestTerm]->GetNearestWord();
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
        Post *NextDocument( ISREndDoc* docEnd  ) override
            {
            // Seek all the ISRs to the first occurrence just past
            // the end of this document.
            return Seek( (docEnd->GetEndLocation() == 0) ? 0 : docEnd->GetEndLocation() + 1, docEnd );
            }

        Post *Seek( Location target, ISREndDoc* docEnd ) override;

        Post * Next( ISREndDoc* docEnd ) override
            {
            return Seek( nearestStartLocation + 1, docEnd );
            }

        APESEARCH::string GetNearestWord() override 
            {
            return terms[nearestTerm]->GetNearestWord();
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
        ~ISRContainer() {
            for (int i = 0; i < countContained; ++i) {
                delete contained[i];
            }
            delete contained;
        }

        ISR **contained; //List of ISRs to include
        ISR *excluded; //ISR to exclude

        unsigned countContained, countExcluded;
        // Location Next( );
        
        Post *Seek( Location target, ISREndDoc* docEnd ) override
            {
            // 1. Seek all the included ISRs to the first occurrence beginning at
            //    the target location.
            
            // 2. Move the document end ISR to just past the furthest
            //    contained ISR, then calculate the document begin location.

            // 3. Seek all the other contained terms to past the document begin.

            // 4. If any contained term is past the document end, return to
            //    step 2.

            // 5. If any ISR reaches the end, there is no match.


            // nearestStartLocation = endDocLoc;
            // nearestEndLocation = target;
            // for (int i = 0; i < countContained; ++i) {\
            //     // If one of the terms doesn't have a posting list, this document isn't a match
            //     if (contained[i] == nullptr) return nullptr;

            //     Post* foundPost = contained[i]->Seek(target, endDocLoc);
            //     if ((!foundPost) || foundPost->loc > endDocLoc) return nullptr;
            //     if (foundPost->loc >= nearestEndLocation)
            //         {
            //         nearestEndLocation = foundPost->loc;
            //         farthestTerm = i;
            //         }
            //     if (foundPost->loc <= nearestStartLocation)
            //         {
            //         post = foundPost;
            //         nearestContained = i;
            //         nearestStartLocation = foundPost->loc;
            //         nearestTerm = i;
            //         }
            // }

            // // 6. Seek all the excluded ISRs to the first occurrence beginning at
            // //    the document begin location.
            // if (excluded) {
            //     excluded->Seek(target, endDocLoc);
            //     Location loceroni = excluded->GetStartLocation();
            //     return (loceroni < endDocLoc) ? post : nullptr;
            // }

            // // 7. If any excluded ISR falls within the document, reset the
            // //    target to one past the end of the document and return to
            // //    step 1.
            // return post;
            }

        Post * Next( ISREndDoc* docEnd ) override
                {
                // Post* nearestNextPost = contained[nearestTerm]->Next(endDocLoc);
                
                // Location minLocation = endDocLoc;
                // // Return the new nearest match.
                // for (int i = 0; i < countContained; ++i) {
                //     if (!contained[i]) continue;
                //     Location currentLoc = contained[i]->GetStartLocation();

                //     if (currentLoc < minLocation ) {
                //         nearestTerm = i;
                //         nearestStartLocation = currentLoc;
                //         minLocation = currentLoc;
                //     }  
                // }

                // Post *newNearest = contained[nearestTerm]->Seek(minLocation, endDocLoc);
                // if (!newNearest) nearestStartLocation = endDocLoc;
                // else if (newNearest->loc > endDocLoc) {
                //     nearestStartLocation = endDocLoc;
                //     return nullptr;
                // }
                
                // return newNearest;
                }

        Location GetStartLocation() override
            {
                return nearestStartLocation;
            }

        Location GetEndLocation() override
            {
                return nearestEndLocation;
            }

        Post *NextDocument( ISREndDoc* docEnd ) override
            {
            // Seek all the ISRs to the first occurrence just past
            // the end of this document.
            return Seek( docEnd->GetStartLocation() + 1, docEnd);
            }

        APESEARCH::string GetNearestWord() override {return contained[nearestContained]->GetNearestWord();}

    private:
        Post * post;

        unsigned nearestTerm, farthestTerm;
        unsigned nearestContained;
        Location nearestStartLocation, nearestEndLocation;
    };

#endif