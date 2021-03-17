#include <unistd.h>
#include "ISR.h"
#include <string>
#include "Token.h"

#ifndef _TUPLE_H
#define _TUPLE_H

class Tuple {
    public:
        Tuple* next; //next pointer in linked list
        virtual ISR* Compile();
        Tuple();
        virtual ~Tuple(); //deallocate node from heap
};

class SearchWord : public Tuple {
    public:
        SearchWord();
        ISR* Compile() override;
        
        std::string word;
};

class SimpleConstraint : public Tuple {
    public:
        SimpleConstraint();
        ISR* Compile() override;

        Tuple* actualConstraint;
};

class UnarySimpleConstraint : public Tuple {
    public:
        UnarySimpleConstraint();
        ISR* Compile() override;

        TokenType op;
        Tuple* actualConstraint;
};

#endif