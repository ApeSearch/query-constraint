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
    auto curr = Top;
    while (curr != nullptr)
        {
            curr->Compile();
            curr = curr->next;
        }
    }

AndExpression::AndExpression() : TupleList() {}

ISR* AndExpression::Compile() {
    auto curr = Top;
    while (curr != nullptr)
        {
            curr->Compile();
            curr = curr->next;
        }
}

Phrase::Phrase() : TupleList() {}

ISR* Phrase::Compile() {
    auto curr = Top;
    while (curr != nullptr)
        {
            curr->Compile();
            curr = curr->next;
        }
}

