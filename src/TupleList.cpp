#include "../include/TupleList.h"
#include "assert.h"
#include <iostream>

using namespace query;

TupleList::TupleList(): Top(nullptr), Bottom(nullptr) {}

TupleList::~TupleList() {
    // if (Top == Bottom) return;
    // Tuple* current = Top;
    // while( current != 0 ) {
    //     Tuple* next = current->next;
    //     delete current;
    //     current = next;
    // }
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

OrExpression::~OrExpression() {
}

ISR* OrExpression::Compile(IndexHT *indexPtr) 
    {
    ISROr* orISR = new ISROr(indexPtr);

    Tuple* curr = Top;
    while (curr != nullptr)
        {
            orISR->numTerms++;
            curr = curr->next;
        }
    
    curr = Top;
    orISR->terms = new ISR*[orISR->numTerms];

    // int failed = 0;
    for(auto i = orISR->terms; i < orISR->terms + orISR->numTerms; ++i)
        {
            *i = curr->Compile(indexPtr);
            auto next = curr->next;
            delete curr;
            curr = next;
        }

    return orISR;
    }

AndExpression::AndExpression() : TupleList() {}

ISR* AndExpression::Compile(IndexHT *indexPtr) {

    ISRAnd* andISR = new ISRAnd(indexPtr);

    Tuple* curr = Top;
    while (curr != nullptr)
        {
            andISR->numTerms++;
            curr = curr->next;
        }
    
    curr = Top;
    andISR->terms = new ISR*[andISR->numTerms];
    
    for(auto i = andISR->terms; i < andISR->terms + andISR->numTerms; ++i) 
        {
            *i = curr->Compile(indexPtr);
            auto next = curr->next;
            delete curr;
            curr = next;
        }
        

    return andISR;
}

Phrase::Phrase() : TupleList() {}

ISR* Phrase::Compile(IndexHT *indexPtr) {
    ISRPhrase* phraseISR = new ISRPhrase(indexPtr);

    Tuple* curr = Top;

    while (curr != nullptr)
        {
            phraseISR->numTerms++;
            curr = curr->next;
        }
    
    curr = Top;
    phraseISR->terms = new ISR*[phraseISR->numTerms];

    for(auto i = phraseISR->terms; i < phraseISR->terms + phraseISR->numTerms; ++i)
        {
            *i = curr->Compile(indexPtr);
            auto next = curr->next;
            delete curr;
            curr = next;
        }
        



    return phraseISR;
}

