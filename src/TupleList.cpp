#include "../include/TupleList.h"
#include "assert.h"
#include <iostream>

using namespace query;

ISR* compileHelper(ISRAnd* toCompile, query::Tuple* top, const IndexBlob* indexPtr);

TupleList::TupleList(): Top(nullptr), Bottom(nullptr) {}

TupleList::~TupleList() {
    // if (Top == Bottom) return;
    // Tuple* current = Top;
    // while( current != 0 ) {
    //     Tuple* next = current->next;
    //     delete current;
    //     current = next;
    // }
}

void TupleList::Empty()
    {
        Bottom = Top = nullptr;
    }

void TupleList::Append(query::Tuple* t)
    {
        // t->next = nullptr;
        if(!Top){
            Top = Bottom = t;
        }
        
        else{
            Bottom->next = t;
            Bottom = t;
        }
    }

OrExpression::OrExpression() : TupleList() {}

OrExpression::~OrExpression() {
}

// OR Compiler
ISR* OrExpression::Compile(const IndexBlob *indexPtr) 
    {
    ISROr* orISR = new ISROr(indexPtr);
    return (ISROr *) compileHelper((ISRAnd *) orISR, Top, indexPtr);
    } // end OrExpression::Compile()

AndExpression::AndExpression() : TupleList() {}

// AND Compiler
ISR* AndExpression::Compile(const IndexBlob *indexPtr) {
    ISRContainer* andISR = new ISRContainer(indexPtr);  
    query::Tuple* curr = Top;

    APESEARCH::vector<query::Tuple*> include;
    APESEARCH::vector<query::Tuple*> exclude;

    int counter = 0;
    while (curr != nullptr)
        {   
            query::UnarySimpleConstraint * isUnary = dynamic_cast<query::UnarySimpleConstraint *>(curr);
            if (isUnary)
                {
                exclude.push_back(curr);
                andISR->countExcluded++;
                }
            else
                {
                include.push_back(curr);
                andISR->countContained++;
                }

            ++counter;
            curr = curr->next;
        }
    
    curr = Top;
    andISR->contained = new ISR*[andISR->countContained];
    andISR->excluded = new ISR*[andISR->countExcluded];

    // for(auto i = andISR->contained; i < andISR->contained + andISR->countContained; ++i)
    //     {
    //         *i = curr->Compile(indexPtr);
    //         auto next = curr->next;
    //         delete curr;
    //         curr = next;
    //     }

    for (int i = 0; i < include.size(); ++i) {
        andISR->contained[i] = include[i]->Compile(indexPtr);
        delete include[i];
    }

    for (int i = 0; i < exclude.size(); ++i) {
        andISR->excluded[i] = exclude[i]->Compile(indexPtr);
        delete exclude[i];
    }
        
    return andISR;
} // end AndExpression::Compile()

Phrase::Phrase() : TupleList() {}

// Phrase Compiler
ISR* Phrase::Compile(const IndexBlob *indexPtr) {
    ISRPhrase* phraseISR = new ISRPhrase(indexPtr);   
    return (ISRPhrase *) compileHelper((ISRAnd *) phraseISR, Top, indexPtr);
}

ISR* compileHelper(ISRAnd* toCompile, query::Tuple* top, const IndexBlob* indexPtr) {
    query::Tuple* curr = top;
    while (curr != nullptr)
        {
            toCompile->numTerms++;
            curr = curr->next;
        }
    
    curr = top;
    toCompile->terms = new ISR*[toCompile->numTerms];

    for(auto i = toCompile->terms; i < toCompile->terms + toCompile->numTerms; ++i)
        {
            *i = curr->Compile(indexPtr);
            auto next = curr->next;
            delete curr;
            curr = next;
        }
        
    return toCompile;
}
