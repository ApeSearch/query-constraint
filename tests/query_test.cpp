#include <vector>
#include <string>
#include <iostream>

#include "../include/QueryParser.h"

std::vector<std::string> queries = {
    // "GET /q= HTTP/1.1", // <empty query>
    // "GET /q=the HTTP/1.1", // the

    "GET /q=%7C HTTP/1.1", // |

    // "GET /q=the%20&%20quick HTTP/1.1", // the & quick
    "GET /q=the%20%7C%20quick HTTP/1.1", // the | quick
    "GET /q=the%20%7C%7C%20quick HTTP/1.1", // the || quick

    // "GET /q=the%20&quick HTTP/1.1", // the &quick
    // "GET /q=the&quick HTTP/1.1", // the&quick
    // "GET /q=the&%20quick HTTP/1.1", // the& quick

    "GET /q=the%20%7C%7Cquick HTTP/1.1", // the |quick
    "GET /q=the%20%7Cquick HTTP/1.1", // the ||quick
    "GET /q=the%7Cquick HTTP/1.1", // the|quick
    "GET /q=the%7C%20quick HTTP/1.1", // the| quick
    "GET /q=the%7C%7C%20quick HTTP/1.1", // the|| quick

    "GET /q=the%7C%7C HTTP/1.1", // the||
    "GET /q=the%20%7C%7C HTTP/1.1", // the ||

    // "GET /q=the%20quick%20brown%20fox HTTP/1.1", //
    // "GET /q=the.quick....brown..fox HTTP/1.1", // the.quick....brown..fox
    // "GET /q=%22the%20quick%20brown%20fox%22 HTTP/1.1", // 
    // "GET /q=(the%20quick)%20brown%20fox HTTP/1.1", // 
    // "GET /q=the%20-quick%20brown%20fox HTTP/1.1", // 
    // "GET /q=the%20quick%20&&%20brown%20fox HTTP/1.1", // 
    // "GET /q=the%20quick%20&%20brown%20fox HTTP/1.1", // 
    // "GET /q=the%20quick%20%7C%7C%7C%20brown%20fox HTTP/1.1", // 
};

int main() 
    {
        for (auto queryLine : queries) 
            {
                auto query = QueryParser(queryLine);
                while (query.FindNextToken()->getTokenType() != TokenType::TokenTypeEOF);
                std::cout << "==========\n";
            }
        
        // auto query = QueryParser(queries[0]);

        // TupleList* orConstraint = query.FindOrConstraint();

        // TupleList* AndConstraint = ( TupleList* ) orConstraint->Top;

        // SearchWord* curr = (SearchWord * ) AndConstraint->Top;
        // while(curr != nullptr){
        //     std::cout << curr->word << std::endl;
        //     curr = (SearchWord * ) curr->next;
        // }
        
    }