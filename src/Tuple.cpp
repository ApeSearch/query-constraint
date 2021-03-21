#include "../include/Tuple.h"
#include "assert.h"
#include <string>

Tuple::Tuple() : next(nullptr) {}

Tuple::~Tuple() {

}

// End Tuple

SearchWord::SearchWord(std::string _word) : Tuple(), word(_word) {}

SearchWord::~SearchWord() {}

ISR* SearchWord::Compile() {
    std::cout << word << std::endl;
    // TODO: unique ptrs
    return new ISRWord(word);
}

// End SearchWord

SimpleConstraint::SimpleConstraint() : Tuple(), actualConstraint(nullptr) {}

SimpleConstraint::~SimpleConstraint() {}

ISR* SimpleConstraint::Compile() {}

// End SimpleConstraint

UnarySimpleConstraint::UnarySimpleConstraint() : Tuple(), op(), actualConstraint(nullptr) {}

UnarySimpleConstraint::~UnarySimpleConstraint() {}

ISR* UnarySimpleConstraint::Compile() {}

// End UnarySimpleConstraint

//NestedConstraint::NestedConstraint(Tuple* tup): Tuple(), constraint(tup) {}

//Implement ISR constructors before Compile for each Tuple
