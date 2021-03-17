#include <unistd.h>
#include "ISR.h"
#include "Tuple.h"

#ifndef _TUPLELIST_H
#define _TUPLELIST_H

class TupleList : public Tuple {
    public:
        Tuple *Top, *Bottom; //List of tuples, linked list
        void Empty();
        void Append(Tuple *t); //Add on a tuple
        TupleList();
        ~TupleList(); //deallocate nodes from heap, call destructor for each tuple in list
};

class OrExpression : public TupleList {
    public:
        OrExpression();
};

class AndExpression : public TupleList {
    public:
        AndExpression();
};

class Phrase : public TupleList {
    public:
        Phrase();
};

class NestedConstraint : public OrExpression {
    public:
        NestedConstraint();
};

    

#endif