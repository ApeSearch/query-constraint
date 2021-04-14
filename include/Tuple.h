#include <unistd.h>
#include "ISR.h"
#include "../libraries/AS/include/AS/string.h"
#include "Token.h"

#ifndef _TUPLE_H
#define _TUPLE_H


namespace query{
    
    class Tuple {
        public:
            Tuple* next; //next pointer in linked list
            virtual ISR* Compile(IndexHT *indexPtr) = 0;
            Tuple();
            virtual ~Tuple(); //deallocate node from heap
    };

    class SearchWord : public Tuple {
        public:
            SearchWord(APESEARCH::string _word);
            ~SearchWord();
            ISR* Compile(IndexHT *indexPtr) override;

            APESEARCH::string word;
    };

    class UnarySimpleConstraint : public Tuple {
        public:
            UnarySimpleConstraint(Tuple* tup);
            ~UnarySimpleConstraint();
            ISR* Compile(IndexHT *indexPtr) override;

            TokenType op;
            Tuple* actualConstraint;
    };

    class NestedConstraint : public Tuple {
        public:
            NestedConstraint(Tuple* tup);
            ~NestedConstraint();
            ISR* Compile(IndexHT *indexPtr) override;

            Tuple* constraint;
    };
}


#endif