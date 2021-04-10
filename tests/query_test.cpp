#include "queries.h"
#include "../include/QueryParser.h"

int main() 
    {
    auto queryLine = queries[19];
    std::cout << queryLine << "\n========\n";
    auto query = QueryParser(queryLine);

    Tuple* orConstraint = query.FindOrConstraint();
    
    if(!orConstraint)
        return 0;
    
    ISR* testTree = orConstraint->Compile(nullptr);
    delete orConstraint;
    }