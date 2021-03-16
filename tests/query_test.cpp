#include <vector>
#include <string>
#include <iostream>

#include "../include/QueryParser.h"

std::vector<std::string> queries = {
    // "GET /q= HTTP/1.1",
    // "GET /q=the HTTP/1.1",
    "GET /q=the%20quick%20brown%20fox HTTP/1.1",
    // "GET /q=%22the%20quick%20brown%20fox%22 HTTP/1.1",
    // "GET /q=(the%20quick)%20brown%20fox HTTP/1.1",
    // "GET /q=the%20-quick%20brown%20fox HTTP/1.1",
};

// "the quick - brown - fox"
// "the quick -brown -fox"

// THESE ARE AND EXPRESSIONS
// the quick & brown

// THESE ARE 
// -
// &
// quick &

int main() 
    {
        for (auto queryLine : queries) 
            {
                auto query = QueryParser(queryLine);
                query.FindNextToken();
            }
    }