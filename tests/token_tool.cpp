#include <vector>
#include <iostream>

#include "../include/QueryParser.h"
#include "../libraries/AS/include/AS/string.h"

void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2);
std::string urlencode(std::string s);

int main () {
    std::string queryIn;

    std::cout << "Enter a query: ";
    std::getline(std::cin, queryIn);
    std::cout << std::endl;
    
    std::string encodedQuery = urlencode(queryIn);
    std::cout << "Encoded query: " << encodedQuery << std::endl;

    // Add in the extra crap from the get request's header
    std::string queryLine = "GET /q=" + encodedQuery + " HTTP/1.1";

    auto Query = QueryParser(APESEARCH::string(queryLine.c_str()));
    Token* curr;
    std::cout << "Decoded query: " << Query.query << "\n\n";
    
    while ((curr = Query.getCurrentToken())->getTokenType() != TokenTypeEOF) {
        std::cout << Token::printTokenType(curr->getTokenType()) << " " << curr->TokenString() << std::endl;
        Query.FindNextToken();
    }
    std::cout << Token::printTokenType(TokenTypeEOF) << std::endl;
    std::cout << "=================================================\n";
}

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

std::string urlencode(std::string s)
    {
    const char *str = s.c_str();
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

    return std::string(v.cbegin(), v.cend());
    }