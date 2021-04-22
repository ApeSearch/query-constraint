
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

// class IndexBlob;
class ListIterator;


class ISR //fix inheritance to be logical, remove duplicate code and member variables
    {

    public:
        ISR();
        ISR(const IndexBlob *_indexPtr);
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

        virtual void Flatten(APESEARCH::vector<ISR *> &flattened) = 0;
        virtual void FlattenStructure(APESEARCH::vector<APESEARCH::vector<ISR *>> &flattened) = 0;

        const IndexBlob *indexPtr;
    };

class ISRWord : public ISR
    {
    public:
        ISRWord();
        ISRWord(ListIterator * plIterator, const IndexBlob *indexPtr, APESEARCH::string word);

        ~ISRWord();

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

        void Flatten(APESEARCH::vector<ISR *> &flattened) override
            {
            flattened.push_back(this);
            }

        void FlattenStructure(APESEARCH::vector<APESEARCH::vector<ISR *>> &flattened)
            {
            flattened.back().push_back(this);
            }

        APESEARCH::string word;
        ListIterator * posts;
        Location startLocation, endLocation;

        unsigned postIndex;
    };

class ISREndDoc : public ISRWord
    {
    public:
        ISREndDoc();
        ISREndDoc(ListIterator * _posts, const IndexBlob *indexPtr);
        ISREndDoc(ListIterator * plIterator);

        size_t GetDocumentLength( );
        unsigned GetTitleLength( );

        Post *Next( ISREndDoc* docEnd ) override;
        Post *Seek( Location target, ISREndDoc* docEnd ) override;

        APESEARCH::string GetNearestWord() override 
            {
                return word;
            }

        void Flatten(APESEARCH::vector<ISR *> &flattened)
            {
            }

        void FlattenStructure(APESEARCH::vector<APESEARCH::vector<ISR *>> &flattened) {}
    };

class ISROr : public ISR
    {
        public:
            ISROr();
            ISROr(const IndexBlob *_indexPtr);
            ~ISROr() {
                for (unsigned int i = 0; i < numTerms; ++i) {
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

            void Flatten(APESEARCH::vector<ISR *> &flattened) override
                {
                // for (unsigned i : foundPosts)
                //     {
                //     terms[i]->Flatten(flattened);
                //     }
                for (unsigned int i = 0; i < numTerms; ++i)
                    {
                    if (terms[i]) 
                        {
                        terms[i]->Flatten(flattened);
                        }
                    }
                }

            void FlattenStructure(APESEARCH::vector<APESEARCH::vector<ISR *>> &flattened)
                {
                for (unsigned int i = 0; i < numTerms; ++i)
                    {   
                    flattened.push_back({});
                    terms[i]->Flatten(flattened.back());
                    }
                }
        
        private:
            Post * post;

            // APESEARCH::vector<unsigned> foundPosts;

            unsigned nearestTerm;
            Location nearestStartLocation, nearestEndLocation;
    };

class ISRAnd : public ISR 
    {
        public:
            ISRAnd();
            ISRAnd(const IndexBlob *_indexPtr);
            ~ISRAnd() {
                for (unsigned int i = 0; i < numTerms; ++i) {
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
            
            void Flatten(APESEARCH::vector<ISR *> &flattened) override
                {
                // for (unsigned i : foundPosts)
                //     {
                //     terms[i]->Flatten(flattened);
                //     }
                for (unsigned int i = 0; i < numTerms; ++i)
                    {
                    if (terms[i]) 
                        {
                        terms[i]->Flatten(flattened);
                        }
                    }
                }

            void FlattenStructure(APESEARCH::vector<APESEARCH::vector<ISR *>> &flattened)
                {
                for (unsigned int i = 0; i < numTerms; ++i)
                    {   
                    terms[i]->FlattenStructure(flattened);
                    // terms[i]->Flatten(flattened.back());
                    }
                }

        private: 
            Post *post;

            // APESEARCH::vector<unsigned> foundPosts;

            unsigned nearestTerm, farthestTerm;
            Location nearestStartLocation, nearestEndLocation;
    };

class ISRPhrase : public ISR
    {
    public:
        ISRPhrase();
        ISRPhrase(const IndexBlob *_indexPtr);
        ~ISRPhrase() {
            for (unsigned int i = 0; i < numTerms; ++i) {
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

        Post * Next( ISREndDoc* docEnd ) override;

        APESEARCH::string GetNearestWord() override 
            {
            return terms[nearestTerm]->GetNearestWord();
            }

        void Flatten(APESEARCH::vector<ISR *> &flattened) override
            {
            // for (unsigned i : foundPosts)
            //     {
            //     terms[i]->Flatten(flattened);
            //     }
            for (unsigned int i = 0; i < numTerms; ++i)
                {
                if (terms[i]) 
                    {
                    terms[i]->Flatten(flattened);
                    }
                }
            }
        
        void FlattenStructure(APESEARCH::vector<APESEARCH::vector<ISR *>> &flattened)
            {
            for (unsigned int i = 0; i < numTerms; ++i)
                {   
                // flattened.push_back({});
                terms[i]->Flatten(flattened.back());
                }
            }
            

    private: 
        Post * post;

        // APESEARCH::vector<unsigned> foundPosts;

        unsigned nearestTerm, farthestTerm;
        Location nearestStartLocation, nearestEndLocation;
    };

class ISRContainer : public ISR
    {
    public:
        ISRContainer();
        ISRContainer(const IndexBlob *_indexPtr);
        ~ISRContainer() {
            for (unsigned int i = 0; i < countContained; ++i) {
                delete contained[i];
            }
            for (unsigned int i = 0; i < countExcluded; ++i) {
                delete excluded[i];
            }
            delete [] contained;
            delete [] excluded;
        }

        ISR **contained; //List of ISRs to include
        ISR **excluded; //ISR to exclude

        unsigned countContained, countExcluded;
        // Location Next( );
        
        Post *Seek( Location target, ISREndDoc* docEnd ) override;

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
            return Seek( (docEnd->GetEndLocation() == 0) ? 0 : docEnd->GetEndLocation() + 1, docEnd);
            }

        APESEARCH::string GetNearestWord() override 
            {
            return contained[nearestContained]->GetNearestWord();
            }

        void Flatten(APESEARCH::vector<ISR *> &flattened) override
            {
            for (unsigned int i = 0; i < countContained; ++i)
                {
                if (contained[i]) 
                    {
                    contained[i]->Flatten(flattened);
                    }
                }

            for (unsigned int i = 0; i < countExcluded; ++i)
                {
                if (excluded[i]) 
                    {
                    excluded[i]->Flatten(flattened);
                    }
                }
            }

        void FlattenStructure(APESEARCH::vector<APESEARCH::vector<ISR *>> &flattened)
            {
            if (!flattened.size())
                {
                flattened.push_back({});
                }
            for (unsigned int i = 0; i < countContained; ++i)
                {   
                // flattened.push_back({});
                // contained[i]->FlattenStructure(flattened);
                contained[i]->Flatten(flattened.back());
                }
            }

    private:
        Post * post;

        unsigned nearestTerm, farthestTerm;
        unsigned nearestContained;
        Location nearestStartLocation, nearestEndLocation;
    };

#endif