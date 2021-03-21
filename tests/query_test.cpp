#include "queries.h"
#include "../include/QueryParser.h"

int main() 
    {
    auto queryLine = queries[0];
    std::cout << queryLine << "\n========\n";
    auto query = QueryParser(queryLine);

    TupleList* orConstraint = query.FindOrConstraint();
    orConstraint->Compile();
    }