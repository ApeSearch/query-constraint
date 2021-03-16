#include <unistd.h>
#include "ISR.h"

#ifndef _TUPLE_H
#define _TUPLE_H

class Tuple {
    public:
        Tuple* next; //next pointer in linked list
        virtual ISR* Compile();
        Tuple();
        virtual ~Tuple(); //deallocate node from heap
};

class NestedConstraint : public Tuple {

};

class SearchWord : public Tuple {

};

class SimpleConstraint : public Tuple {

};

class UnarySimpleConstraint : public Tuple {

};

#endif