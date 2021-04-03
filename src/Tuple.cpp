#include "../include/Tuple.h"
#include "assert.h"
#include "../libraries/AS/include/AS/string.h"
#include <iostream>

using namespace query;

Tuple::Tuple() : next(nullptr) {}

Tuple::~Tuple() {

}

// End Tuple

SearchWord::SearchWord(APESEARCH::string _word) : Tuple(), word(_word) {}

SearchWord::~SearchWord() {}

ISR* SearchWord::Compile() {
    // TODO: unique ptrs
    std::cout << word << std::endl;
    return new ISRWord(word);
}

// End SearchWord

UnarySimpleConstraint::UnarySimpleConstraint(Tuple* tup) : Tuple(), op(), actualConstraint(tup) {}

UnarySimpleConstraint::~UnarySimpleConstraint() {}

ISR* UnarySimpleConstraint::Compile() {}

// End UnarySimpleConstraint

NestedConstraint::NestedConstraint(Tuple* tup): Tuple(), constraint(tup) {}

NestedConstraint::~NestedConstraint() {}

ISR* NestedConstraint::Compile() {}

//Implement ISR constructors before Compile for each Tuple
