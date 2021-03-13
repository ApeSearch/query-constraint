#include "assert.h"
#include <unistd.h>
#include <utility>
#include <iostream>
#include <string>

#ifndef _ISR_H
#define _ISR_H

class ISRWord;

typedef size_t Location;
typedef size_t FileOffset;

enum WordAttributes 
    {
        WordAttributeBold, WordAttributeHeading, WordAttributeLarge
    };

enum DocumentAttributes 
    {
        DocAttributeURL, DocAttributeNumWords, DocAttributeTitle, DocAttributeBody, DocAttributeAnchor
    };

typedef union Attributes
    {
    WordAttributes Word;
    DocumentAttributes Document;
    };

class Post
    {
    public:
        virtual Location GetStartLocation( );
        virtual Location GetEndLocation( );
        virtual Attributes GetAttributes( );
    };

class PostingList
    {
    public:
        virtual Post *Seek( Location );

    private:
        struct PostingListIndex
            {
            FileOffset Offset;
            Location PostLocation;
            };
        PostingListIndex *index;
        virtual char *GetPostingList( );
    };

class Index
    {
    public:
        Location WordsInIndex,
                 DocumentsInIndex,
                 LocationsInIndex,
                 MaximumLocation;

        ISRWord *OpenISRWord( char *word );
        ISRWord *OpenISREndDoc( );
    };

class ISR
    {
    public:
        virtual Post *Next( );
        virtual Post *NextDocument( );
        virtual Post *Seek( Location target );
        virtual Location GetStartLocation( );
        virtual Location GetEndLocation( );
    };

class ISRWord : public ISR
    {
    public:
        unsigned GetDocumentCount( );
        unsigned GetNumberOfOccurrences( );
        virtual Post *GetCurrentPost( );
    };

class ISREndDoc : public ISRWord
    {
    public:
        unsigned GetDocumentLength( );
        unsigned GetTitleLength( );
        unsigned GetUrlLength( );
    };

class ISROr : public ISR
    {
        public:
            ISR **terms;
            unsigned numTerms;

            Location GetStartLocation()
                {
                    return nearestStartLocation;
                }

            Location GetEndLocation()
                {
                    return nearestEndLocation;
                }

            Post *Seek( Location target )
                {
                // Seek all the ISRs to the first occurrence beginning at
                // the target location. Return null if there is no match.
                // The document is the document containing the nearest term.
                }

            Post *Next( )
                {
                // Do a next on the nearest term, then return
                // the new nearest match.
                }

            Post *NextDocument( ) 
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
        ISR **terms;
        unsigned numterms;

        Post *Seek( Location target )
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

        Post * Next()
            {
            return Seek( nearestStartLocation + 1 );
            }

        private: 
            unsigned nearestTerm, farthestTerm;
            Location nearestStartLocation, nearestEndLocation;
    };

class ISRPhrase : public ISR
    {
    public:
        ISR **terms;
        unsigned numterms;

        Post *Seek( Location target )
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

        Post * Next()
            {
            // return Seek( nearestStartLocation + 1 );
            }
    };

class ISRContainer : public ISR
    {
    public:
        ISR **contained;
        ISR *excluded;
        ISREndDoc *endDoc;
        unsigned countContained, countExcluded;
        // Location Next( );
        
        Post *Seek( Location target )
            {
            // 1. Seek all the included ISRs to the first occurrence beginning at
            //    the target location.
            
            // 2. Move the document end ISR to just past the furthest
            //    contained ISR, then calculate the document begin location.

            // 3. Seek all the other contained terms to past the document begin.

            // 4. If any contained ermis past the document end, return to
            //    step 2.

            // 5. If any ISR reaches the end, there is no match.

            // 6. Seek all the excluded ISRs to the first occurrence beginning at
            //    the document begin location.// 7. If any excluded ISR falls within the document, reset the
            //    target to one past the end of the document and return to
            //    step 1.
            }

        Post *Next( ) 
            {
            // Seek( contained[ nearestContained ]->GetStartlocation( ) + 1 );
            }

    private:
        unsigned nearestTerm, farthestTerm;
        Location nearestStartLocation, nearestEndLocation;
    };

#endif