#include "../include/TupleList.h"
#include "assert.h"


Tuple::Tuple() : next(nullptr) {}

SearchWord::SearchWord() : Tuple(), word() {}

SimpleConstraint::SimpleConstraint() : Tuple(), actualConstraint(nullptr) {}

UnarySimpleConstraint::UnarySimpleConstraint() : Tuple(), op(), actualConstraint(nullptr) {}

//Implement ISR constructors before Compile for each Tuple