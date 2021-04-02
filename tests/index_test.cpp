#include "queries.h"
#include "../include/Index.h"
#include "../libraries/AS/include/AS/string.h"
#include <iostream>

int main() 
    {
        APESEARCH::vector<APESEARCH::string> words;
        words.push_back("the");
        words.push_back("cow");
        words.push_back("the");
        words.push_back("pig");
        words.push_back("and");
        words.push_back("all");
        words.push_back("of");
        words.push_back("the");
        words.push_back("animals");


        IndexHT index;
        index.addDoc("https://eecs440.com", words, 10);

        APESEARCH::string s = "the";
        
        hash::Tuple<const char *, WordPostingList *> * entry = index.chunk.Find(s.cstr());

        //WordPostingList* pl = entry->value;
        std::cout << index.chunk.numOfLinkedLists() << std::endl;
    }