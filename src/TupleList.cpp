#include "../include/TupleList.h"
#include "assert.h"


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

ISR* OrExpression::Compile() {}

AndExpression::AndExpression() : TupleList() {}

ISR* AndExpression::Compile() {}

Phrase::Phrase() : TupleList() {}

ISR* Phrase::Compile() {}

