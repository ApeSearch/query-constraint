#include "../include/TupleList.h"
#include "assert.h"


TupleList::TupleList(): Top(nullptr), Bottom(nullptr) {}

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

OrExpression::OrExpression() : TupleList() {}

AndExpression::AndExpression() : TupleList() {}

Phrase::Phrase() : TupleList() {}

