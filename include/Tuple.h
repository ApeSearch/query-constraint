#include <unistd.h>
#include "ISR.h"
#include <string>
#include "Token.h"

#ifndef _TUPLE_H
#define _TUPLE_H

class Tuple {
    public:
        Tuple* next; //next pointer in linked list
        virtual ISR* Compile() = 0;
        Tuple();
        virtual ~Tuple(); //deallocate node from heap

        
};

class SearchWord : public Tuple {
    public:
        SearchWord(std::string _word);
        ~SearchWord();
        ISR* Compile() override;

        std::string word;
};

class SimpleConstraint : public Tuple {
    public:
        SimpleConstraint();
        ~SimpleConstraint();
        ISR* Compile() override;

        Tuple* actualConstraint;
};

class UnarySimpleConstraint : public Tuple {
    public:
        UnarySimpleConstraint();
        ~UnarySimpleConstraint();
        ISR* Compile() override;

        TokenType op;
        Tuple* actualConstraint;
};

/*class NestedConstraint : public Tuple {
    public:
        NestedConstraint(Tuple* tup);
        ~NestedConstraint() {}
        ISR* Compile() override;

        Tuple* constraint;
};*/


#endif