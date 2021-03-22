#include "../include/TupleList.h"
#include "assert.h"
#include <iostream>


TupleList::TupleList(): Top(nullptr), Bottom(nullptr) {}

TupleList::~TupleList() {}

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

OrExpression::OrExpression() : TupleList() 
    {
        
    }

ISR* OrExpression::Compile() 
    {
    ISROr* orISR = new ISROr();

    Tuple* curr = Top;
    while (curr != nullptr)
        {   
            orISR->terms[orISR->numTerms++] = curr->Compile();
            curr = curr->next;
        }

    return orISR;
    }

AndExpression::AndExpression() : TupleList() {}

ISR* AndExpression::Compile() {

    ISRAnd* andISR = new ISRAnd();

    Tuple* curr = Top;
    while (curr != nullptr)
        {
            andISR->numTerms++;
            curr = curr->next;
        }
    
    curr = Top;
    andISR->terms = new ISR*[andISR->numTerms];
        
    for(auto i = andISR->terms; i < andISR->terms + andISR->numTerms; ++i, curr = curr->next)
        *i = curr->Compile();

    return andISR;
}

Phrase::Phrase() : TupleList() {}

ISR* Phrase::Compile() {

    //ISRPhrase* phraseISR = new ISRPhrase();

    Tuple* curr = Top;
    while (curr != nullptr)
        {
            curr->Compile();
            curr = curr->next;
        }
}

