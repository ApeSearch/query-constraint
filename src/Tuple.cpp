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

ISR* SearchWord::Compile(const IndexBlob *indexPtr) {
    // TODO: Get SearchWord ISR from index
    return indexPtr->getWordISR(word); 
}

// End SearchWord

UnarySimpleConstraint::UnarySimpleConstraint(Tuple* tup) : Tuple(), op(), actualConstraint(tup) {}

UnarySimpleConstraint::~UnarySimpleConstraint() {}

ISR* UnarySimpleConstraint::Compile(const IndexBlob *indexPtr) {
    ISRContainer* containerISR = new ISRContainer(indexPtr);

    ISROr *compiled = (ISROr *) actualConstraint->Compile(indexPtr);
    delete actualConstraint;

    ISR** termList;
    unsigned termCount;
    // jank garbage
    if (compiled->numTerms) {
        std::cout << "or" << std::endl;
        containerISR->countExcluded = compiled->numTerms;
        
    } else {
        std::cout << "container" << std::endl;
        containerISR->countExcluded = ((ISRContainer *) compiled)->countContained;
    }

    containerISR->excluded = compiled;

    // ISRAnd * contained = (ISRAnd *) compiled->terms[0];
        
    return containerISR;
}

// End UnarySimpleConstraint

NestedConstraint::NestedConstraint(Tuple* tup): Tuple(), constraint(tup) {}

NestedConstraint::~NestedConstraint() {}

ISR* NestedConstraint::Compile(const IndexBlob *indexPtr) {
    ISRContainer* containerISR = new ISRContainer(indexPtr);

    
    containerISR->contained = new ISR*;
    *containerISR->contained = constraint->Compile(indexPtr);
    ++containerISR->countContained;

    delete constraint;
        
    return containerISR;
}

//Implement ISR constructors before Compile for each Tuple
