#include "../include/TupleList.h"
#include "assert.h"
#include <iostream>

using namespace query;

TupleList::TupleList(): Top(nullptr), Bottom(nullptr) {}

TupleList::~TupleList() {
    Tuple* current = Top;
    while( current != 0 ) {
        Tuple* next = current->next;
        delete current;
        current = next;
    }
}

void TupleList::Empty()
    {
        Bottom = Top = nullptr;
    }

void TupleList::Append(Tuple* t)
    {
        // t->next = nullptr;
        if(!Top){
            Top = Bottom = t;
        }
        
        else{
            Bottom->next = t;
            Bottom = t;
        }
    }

OrExpression::OrExpression() : TupleList() {}

APESEARCH::unique_ptr<ISR> OrExpression::Compile(IndexHT *indexPtr) 
    {
    APESEARCH::unique_ptr<ISROr> orISR = APESEARCH::unique_ptr<ISROr>(new ISROr(indexPtr));

    Tuple* curr = Top;
    while (curr != nullptr)
        {
            orISR->numTerms++;
            curr = curr->next;
        }
    
    curr = Top;
    orISR->terms = APESEARCH::unique_ptr<ISR*>(new ISR*[orISR->numTerms]);

    int failed = 0;
    for(auto i = orISR->terms.get(); i < orISR->terms.get() + orISR->numTerms; ++i, curr = curr->next)
        {
        APESEARCH::unique_ptr<ISR> compiled = curr->Compile(indexPtr);
            if (compiled) 
                {
                *(i-failed) = compiled.get();
                }
            else ++failed;
        }

    orISR->numTerms -= failed;

    return orISR;
    }

AndExpression::AndExpression() : TupleList() {}

APESEARCH::unique_ptr<ISR> AndExpression::Compile(IndexHT *indexPtr) {

    ISRAnd* andISR = new ISRAnd(indexPtr);

    Tuple* curr = Top;
    while (curr != nullptr)
        {
            andISR->numTerms++;
            curr = curr->next;
        }
    
    curr = Top;
    andISR->terms = new ISR*[andISR->numTerms];
    
    for(auto i = andISR->terms; i < andISR->terms + andISR->numTerms; ++i, curr = curr->next)
        *i = curr->Compile(indexPtr);

    return andISR;
}

Phrase::Phrase() : TupleList() {}

APESEARCH::unique_ptr<ISR> Phrase::Compile(IndexHT *indexPtr) {
    std::cout << "PhraseISR" << std::endl;

    ISRPhrase* phraseISR = new ISRPhrase(indexPtr);

    Tuple* curr = Top;

    while (curr != nullptr)
        {
            phraseISR->numTerms++;
            curr = curr->next;
        }
    
    curr = Top;
    phraseISR->terms = new ISR*[phraseISR->numTerms];

    for(auto i = phraseISR->terms; i < phraseISR->terms + phraseISR->numTerms; ++i, curr = curr->next)
        *i = curr->Compile(indexPtr);

    return phraseISR;
}

