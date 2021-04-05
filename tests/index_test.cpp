#include "queries.h"
#include "../include/Index.h"
#include "../libraries/AS/include/AS/string.h"
#include <iostream>
#include <cassert>

int main() 
    {
        APESEARCH::vector<APESEARCH::string> words;
        words.emplace_back("the");
        words.emplace_back("cow");
        words.emplace_back("the");
        words.emplace_back("pig");
        words.emplace_back("and");
        words.emplace_back("all");
        words.emplace_back("of");
        words.emplace_back("the");
        words.emplace_back("animals");
        
        IndexHT index;
        index.addDoc("https://eecs440.com", words, 9);
        index.addDoc("https://eecs441.com", words, 19);

        
        hash::Tuple<const char *, WordPostingList *> * entry = index.chunk.Find("the");

        WordPostingList* pl = entry->value;
        
        Location absoluteLoc = 0;

        for(size_t i = 0; i < pl->numberOfPosts; ++i){
            absoluteLoc += pl->posts[i]->deltaPrev;
            std::cout << pl->posts[i]->deltaPrev << " " << absoluteLoc << std::endl;
        }

        assert(pl->numberOfPosts == 6);
        assert(index.docEndList.get()->numOfDocs == 2);
        
    }