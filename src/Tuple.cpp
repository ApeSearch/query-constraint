#include "../include/Tuple.h"
#include "assert.h"
#include "../libraries/AS/include/AS/string.h"
#include <iostream>

using namespace query;

query::Tuple::Tuple() : next(nullptr) {}

query::Tuple::~Tuple() {

}

// End Tuple

SearchWord::SearchWord(APESEARCH::string _word) : Tuple(), word(_word) {}

SearchWord::~SearchWord() {}

ISR* SearchWord::Compile(IndexHT *indexPtr) {
    // TODO: Get SearchWord ISR from index
    return indexPtr->getWordISR(word); 
}

// End SearchWord

UnarySimpleConstraint::UnarySimpleConstraint(Tuple* tup) : Tuple(), op(), actualConstraint(tup) {}

UnarySimpleConstraint::~UnarySimpleConstraint() {}

ISR* UnarySimpleConstraint::Compile(IndexHT *indexPtr) {}

// End UnarySimpleConstraint

NestedConstraint::NestedConstraint(Tuple* tup): Tuple(), constraint(tup) {}

NestedConstraint::~NestedConstraint() {}

ISR* NestedConstraint::Compile(IndexHT *indexPtr) {}

//Implement ISR constructors before Compile for each Tuple
