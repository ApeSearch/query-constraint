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

OrExpression::OrExpression() : TupleList() {}

ISR* OrExpression::Compile() 
    {
    std::cout << "ORISR" << std::endl;

    ISROr* orISR = new ISROr();

    Tuple* curr = Top;
    while (curr != nullptr)
        {
            orISR->numTerms++;
            curr = curr->next;
        }
    
    curr = Top;
    orISR->terms = new ISR*[orISR->numTerms];

    for(auto i = orISR->terms; i < orISR->terms + orISR->numTerms; ++i, curr = curr->next)
        *i = curr->Compile();

    }

AndExpression::AndExpression() : TupleList() {}

ISR* AndExpression::Compile() {

    ISRAnd* andISR = new ISRAnd();
    std::cout << "ANDISR" << std::endl;
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
    std::cout << "PhraseISR" << std::endl;

    ISRAnd* phraseISR = new ISRAnd();

    Tuple* curr = Top;

    while (curr != nullptr)
        {
            phraseISR->numTerms++;
            curr = curr->next;
        }
    
    curr = Top;
    phraseISR->terms = new ISR*[phraseISR->numTerms];

    for(auto i = phraseISR->terms; i < phraseISR->terms + phraseISR->numTerms; ++i, curr = curr->next)
        *i = curr->Compile();

    return phraseISR;
}

