#include <unistd.h>
#include "ISR.h"
#include "Tuple.h"

#ifndef _TUPLELIST_H
#define _TUPLELIST_H

class TupleList : public Tuple {
    public:
        Tuple *Top, *Bottom;
        void Empty();
        void Append(Tuple *t);
        TupleList();
        ~TupleList();
};

class OrExpression : public TupleList {

};

class AndExpression : public TupleList {

};

class SimpleConstraint : public TupleList {

};
    

#endif