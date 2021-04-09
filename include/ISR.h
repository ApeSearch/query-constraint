
#pragma once

#ifndef _ISR_H
#define _ISR_H

#include "assert.h"
#include "HashTable.h"
#include <unistd.h>
#include <utility>
#include <iostream>
#include "../libraries/AS/include/AS/string.h"
#include "Index.h"


class ISR //fix inheritance to be logical, remove duplicate code and member variables
    {

    public:
        ISR();
        virtual ~ISR() {}

        // Store information the index provides i.e. the posting list, the location 
        // ( so as to provide information about how to search for the next location or a post assoicated with a location)
        virtual Post *Next( ) = 0;
        virtual Post *NextDocument( ) = 0;
        virtual Post *Seek( Location target ) = 0;
        virtual Location GetStartLocation( ) = 0;
        virtual Location GetEndLocation( ) = 0;

    };

class ISRWord : public ISR
    {
    public:
        ISRWord();
        ISRWord(PostingList * _posts);

        unsigned GetDocumentCount( );
        unsigned GetNumberOfOccurrences( );
        virtual Post *GetCurrentPost( );

        // needed
        Post *Next( ) override;
        Post *NextDocument( ) override;
        Post *Seek( Location target ) override;
        Location GetStartLocation( ) override;
        Location GetEndLocation( ) override;

    private:
        PostingList* posts;
    };

class ISREndDoc : public ISRWord
    {
    public:
        ISREndDoc();
        ISREndDoc(PostingList* _posts);

        unsigned GetDocumentLength( );
        unsigned GetTitleLength( );
        unsigned GetUrlLength( );


    };

class ISROr : public ISR
    {
        public:
            ISROr();

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
                }

            Post *Next( ) override
                {
                // Do a next on the nearest term, then return
                // the new nearest match.
                }

            Post *NextDocument( ) override
                {
                // Seek all the ISRs to the first occurrence just past
                // the end of this document.
                // return Seek( DocumentEnd->GetEndLocation( ) + 1 );
                }
        
        private:
            unsigned nearestTerm;
            Location nearestStartLocation, nearestEndLocation;
    };

class ISRAnd : public ISR 
    {
        public:
            ISRAnd();

            ISR **terms;
            unsigned numTerms;

            Post *Seek( Location target ) override
                {
                    // 1. Seek all the ISRs to the first occurrence beginning at
                    //    the target location.

                    // 2. Move the document end ISR to just past the furthest
                    //    word, then calculate the document begin location.

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
                // return Seek( DocumentEnd->GetEndLocation( ) + 1 );
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
    };

class ISRPhrase : public ISR
    {
    public:
        ISRPhrase();

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
            // return Seek( DocumentEnd->GetEndLocation( ) + 1 );
            }

        Post *Seek( Location target ) override
            {
                // 1. Seek all ISRs to the first occurrence beginning at
                //    the target location.

                // 2. Pick the furthest term and attempt to seek all
                //    the other terms to the first location beginning
                //    where they should appear relative to the furthest
                //    term.

                // 3. If any term is past the desired location, return
                //    to step 2.

                // 4. If any ISR reaches the end, there is no match.
            }

        Post * Next() override
            {
            // return Seek( nearestStartLocation + 1 );
            }
    private: 
        unsigned nearestTerm, farthestTerm;
        Location nearestStartLocation, nearestEndLocation;
    };

class ISRContainer : public ISR
    {
    public:
        ISRContainer();

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
            // Seek( contained[ nearestContained ]->GetStartlocation( ) + 1 );
            }

    private:
        unsigned nearestTerm, farthestTerm;
        Location nearestStartLocation, nearestEndLocation;
    };

#endif