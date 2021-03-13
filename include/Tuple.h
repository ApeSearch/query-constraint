#include <unistd.h>
#include "ISR.h"

#ifndef _TUPLE_H
#define _TUPLE_H

class Tuple {
    public:
        Tuple* next;
        virtual ISR* Compile();
        Tuple();
        virtual ~Tuple();
};

class Phrase : public Tuple {
    
};

class NestedConstraint : public Tuple {

};

class SearchWord : public Tuple {

};


#endif