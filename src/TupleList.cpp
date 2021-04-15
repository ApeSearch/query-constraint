#include "../include/TupleList.h"
#include "assert.h"
#include <iostream>

using namespace query;

ISR* compileHelper(ISRAnd* toCompile, Tuple* top, IndexHT* indexPtr);

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

// OR Compiler
ISR* OrExpression::Compile(IndexHT *indexPtr) 
    {
    ISROr* orISR = new ISROr(indexPtr);
    return (ISROr *) compileHelper((ISRAnd *) orISR, Top, indexPtr);
    } // end OrExpression::Compile()

AndExpression::AndExpression() : TupleList() {}

// AND Compiler
ISR* AndExpression::Compile(IndexHT *indexPtr) {
    ISRAnd* andISR = new ISRAnd(indexPtr);  
    return (ISRAnd *) compileHelper(andISR, Top, indexPtr);
} // end AndExpression::Compile()

Phrase::Phrase() : TupleList() {}

// Phrase Compiler
ISR* Phrase::Compile(IndexHT *indexPtr) {
    ISRPhrase* phraseISR = new ISRPhrase(indexPtr);   
    return (ISRPhrase *) compileHelper((ISRAnd *) phraseISR, Top, indexPtr);
}

ISR* compileHelper(ISRAnd* toCompile, Tuple* top, IndexHT* indexPtr) {
    Tuple* curr = top;
    while (curr != nullptr)
        {
            toCompile->numTerms++;
            curr = curr->next;
        }
    
    curr = top;
    toCompile->terms = new ISR*[toCompile->numTerms];

    for(auto i = toCompile->terms; i < toCompile->terms + toCompile->numTerms; ++i)
        {
            *i = curr->Compile(indexPtr);
            auto next = curr->next;
            delete curr;
            curr = next;
        }
        
    return toCompile;
}
