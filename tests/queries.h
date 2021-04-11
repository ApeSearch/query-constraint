#include <vector>
#include "../libraries/AS/include/AS/string.h"

std::vector<APESEARCH::string> queries = {
    // 0 Basic queries
    "GET /q= HTTP/1.1", // <empty query>
    "GET /q=the HTTP/1.1", // the
    "GET /q=the%20quick%20brown%20fox HTTP/1.1", // the quick brown fox
    "GET /q=the%20&%20quick HTTP/1.1", // the & quick
    "GET /q=the%20&&%20quick HTTP/1.1", // the && quick
    "GET /q=the%20%7C%20quick HTTP/1.1", // the | quick
    "GET /q=the%20%7C%7C%20quick HTTP/1.1", // the || quick

    // NOT queries
    "GET /q=the%20-quick%20brown%20fox HTTP/1.1", // the -quick brown fox
    "GET /q=the%20-quick%20-brown%20fox HTTP/1.1", // the -quick -brown fox
    "GET /q=the%20-quick%20-%22brown%20fox%22 HTTP/1.1", // the -quick -brown fox
    "GET /q=the%20-quick%20--brown%20fox HTTP/1.1", // the -quick --brown fox
    "GET /q=tree%20-%20house HTTP/1.1", // tree - house
    "GET /q=tree%20---%20house HTTP/1.1", // tree --- house
    "GET /q=tree-house HTTP/1.1", // tree-house
    "GET /q=tree---house HTTP/1.1", // tree---house
    "GET /q=tree-%20house HTTP/1.1", // tree- house
    "GET /q=tree---%20house HTTP/1.1", // tree--- house
    "GET /q=tree%20-house HTTP/1.1", // tree -house
    "GET /q=tree%20---house HTTP/1.1", // tree ---house

    // // Nested/Phrase queries    
    "GET /q=%22the%20quick%20brown%20fox%22 HTTP/1.1", // "the quick brown fox"
    "GET /q=(the%20quick)%20brown%20fox HTTP/1.1", // (the quick) brown fox

    // // Edge Cases
    "GET /q=the.quick....brown..fox HTTP/1.1", // the.quick....brown..fox
    "GET /q=the.quick%20....brown..fox HTTP/1.1", // the.quick ....brown..fox
    "GET /q=the,quick,,,,brown,fox HTTP/1.1", // the,quick,,,,brown,fox
    "GET /q=the,quick%20,,,,brown,,fox HTTP/1.1", // the,quick ,,,,brown,,fox
    "GET /q=the;quick;;;;brown;fox HTTP/1.1", // the;quick;;;;brown;fox
    "GET /q=the;quick%20;;;;brown;;%20fox HTTP/1.1", // the;quick ;;;;brown;; fox

    "GET /q=the%20quick%20&&&%20brown%20fox HTTP/1.1", // the quick &&& brown fox
    "GET /q=the%20quick%20%7C%7C%7C%20brown%20fox HTTP/1.1", // the quick ||| brown fox

    // // Operator weirdness
    "GET /q=the%20&quick HTTP/1.1", // the &quick
    "GET /q=the%20&&quick HTTP/1.1", // the &&quick
    "GET /q=the&quick HTTP/1.1", // the&quick
    "GET /q=the&%20quick HTTP/1.1", // the& quick
    "GET /q=the&&%20quick HTTP/1.1", // the&& quick
    "GET /q=the%20%7Cquick HTTP/1.1", // the |quick
    "GET /q=the%20%7C%7Cquick HTTP/1.1", // the ||quick
    "GET /q=the%7Cquick HTTP/1.1", // the|quick
    "GET /q=the%7C%20quick HTTP/1.1", // the| quick
    "GET /q=the%7C%7C%20quick HTTP/1.1", // the|| quick
    "GET /q=the%20%7C%7C HTTP/1.1", // the ||
    "GET /q=the%20&& HTTP/1.1", // the &&

    // Tabs and Newlines
    "GET /q=the\nquick HTTP/1.1", // the\nquick
    "GET /q=the\tquick HTTP/1.1", // the\tquick
    "GET /q=the%20\nquick HTTP/1.1", // the \nquick
    "GET /q=the%20\tquick HTTP/1.1", // the \tquick
    "GET /q=the\n%20quick HTTP/1.1", // the\n quick
    "GET /q=the\t%20quick HTTP/1.1", // the\t quick
// };

//     // Null character
    // "GET /q=the\0quick HTTP/1.1", // the\0quick
// std::vector<std::string> weirdQueries {

    // Just awful
    "GET /q=the%7C&.%7Cquick..&.&%22.brown.%7C&%22.&fox HTTP/1.1", // idek
    "GET /q=the-%7C&-.%7Cquick..&.&%22.brown.%7C&%22.&fox HTTP/1.1", // idek
    "GET /q=th-e%20qu%22ick&%20brown&%7C&fox HTTP/1.1", // idek

    "GET /q=the%7C&%20quick HTTP/1.1", // the|& quick
    "GET /q=the&%7C%20quick HTTP/1.1", // the&| quick
    "GET /q=the%20&%7Cquick HTTP/1.1", // the |&quick
    "GET /q=the%20%7C&quick HTTP/1.1", // the &|quick

    "GET /q=the%20&%7C%20quick HTTP/1.1", // the &| quick
    "GET /q=the%20%7C&%20quick HTTP/1.1", // the |& quick
};

//////////////////////////////////////
// Utilities stolen from stackoverflow
//////////////////////////////////////

void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2)
{
    hex1 = c / 16;
    hex2 = c % 16;
    hex1 += hex1 <= 9 ? '0' : 'a' - 10;
    hex2 += hex2 <= 9 ? '0' : 'a' - 10;
}

APESEARCH::string urlencode(APESEARCH::string s)
    {
    const char *str = s.cstr();
    std::vector<char> v(s.size());
    v.clear();
    for (size_t i = 0, l = s.size(); i < l; i++)
        {
        char c = str[i];
        if ((c >= '0' && c <= '9') ||
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
                c == '*' || c == '\'' || c == '(' || c == ')')
            {
            v.push_back(c);
            }
        else
            {
            v.push_back('%');
            unsigned char d1, d2;
            hexchar(c, d1, d2);
            v.push_back(d1);
            v.push_back(d2);
            }
        }

    return APESEARCH::string(v.cbegin(), v.cend());
    }