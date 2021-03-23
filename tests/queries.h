#include <vector>
#include <string>

std::vector<std::string> queries = {
    // 0 Basic queries
    "GET /q= HTTP/1.1", // <empty query>
    "GET /q=the HTTP/1.1", // the
    "GET /q=the%20quick%20brown%20fox HTTP/1.1", // the quick brown fox
    "GET /q=the%20&%20quick HTTP/1.1", // the & quick
    "GET /q=the%20&&%20quick HTTP/1.1", // the && quick
    "GET /q=the%20%7C%20quick HTTP/1.1", // the | quick
    "GET /q=the%20%7C%7C%20quick HTTP/1.1", // the || quick

    // 7 NOT queries
    "GET /q=the%20-quick%20brown%20fox HTTP/1.1", // the -quick brown fox
    "GET /q=tree%20-%20house HTTP/1.1", // tree - house
    "GET /q=tree-house HTTP/1.1", // tree-house
    "GET /q=tree%20-house HTTP/1.1", // tree -house

    // 9 Nested/Phrase queries    
    "GET /q=%22the%20quick%20brown%20fox%22 HTTP/1.1", // "the quick brown fox"
    "GET /q=(the%20quick)%20brown%20fox HTTP/1.1", // (the quick) brown fox

    // 11 Edge Cases
    "GET /q=the%20%7C%7C HTTP/1.1", // the ||
    "GET /q=the%20&& HTTP/1.1", // the &&

    "GET /q=the.quick....brown..fox HTTP/1.1", // the.quick....brown..fox
    "GET /q=the.quick%20....brown..fox HTTP/1.1", // the.quick ....brown..fox

    "GET /q=the%20quick%20&&&%20brown%20fox HTTP/1.1", // the quick &&& brown fox
    "GET /q=the%20quick%20%7C%7C%7C%20brown%20fox HTTP/1.1", // the quick ||| brown fox

    "GET /q=the%20&quick HTTP/1.1", // the &quick
    "GET /q=the%20&&quick HTTP/1.1", // the &&quick
    "GET /q=the&quick HTTP/1.1", // the&quick
    "GET /q=the&%20quick HTTP/1.1", // the& quick
    "GET /q=the&&%20quick HTTP/1.1", // the& quick

    "GET /q=the%20%7C%7Cquick HTTP/1.1", // the |quick
    "GET /q=the%20%7Cquick HTTP/1.1", // the ||quick
    "GET /q=the%7Cquick HTTP/1.1", // the|quick
    "GET /q=the%7C%20quick HTTP/1.1", // the| quick
    "GET /q=the%7C%7C%20quick HTTP/1.1", // the|| quick
};