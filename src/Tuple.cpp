#include "../include/Tuple.h"
#include "assert.h"
#include <string>


Tuple::Tuple() : next(nullptr) {}

Tuple::~Tuple() {

}

// End Tuple

SearchWord::SearchWord(std::string _word) : Tuple(), word(_word) {}

SearchWord::~SearchWord() {}

ISR* SearchWord::Compile() {}

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
