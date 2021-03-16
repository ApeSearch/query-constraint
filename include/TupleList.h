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

};

class AndExpression : public TupleList {

};

class Phrase : public TupleList {
    
};
    

#endif