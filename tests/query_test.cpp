#include "queries.h"
#include "../include/QueryParser.h"

int main() 
    {
    auto queryLine = queries[2];
    std::cout << queryLine << "\n========\n";
    auto query = QueryParser(queryLine);

    Tuple* orConstraint = query.FindOrConstraint();
    
    if(!orConstraint)
        return 0;
    
    orConstraint->Compile();
    }