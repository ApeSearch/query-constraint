#include <unistd.h>
#include "ISR.h"
#include "Tuple.h"

#include "../libraries/AS/include/AS/vector.h"

#ifndef _TUPLELIST_H
#define _TUPLELIST_H


namespace query {

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
            ~OrExpression();
            ISR* Compile(const IndexBlob *indexPtr);
    };

    class AndExpression : public TupleList {
        public:
            AndExpression();
            ISR* Compile(const IndexBlob *indexPtr);
    };

    class Phrase : public TupleList {
        public:
            Phrase();
            ISR* Compile(const IndexBlob *indexPtr);
    };

}

#endif