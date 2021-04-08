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
        
        IndexHT *index = new IndexHT();
        index->addDoc("https://eecs440.com", words, 9, BodyText);
        index->addDoc("https://eecs441.com", words, 19, TitleText);

        hash::Tuple<APESEARCH::string, PostingList *> * entry = index->dict.Find(APESEARCH::string("the"));
        hash::HashTable<APESEARCH::string, PostingList *>::Iterator itr = index->dict.begin();


        while(itr != index->dict.end()){
            std::cout << itr->key << std::endl;
            itr++;
        }

        /*PostingList* pl = entry->value;

        for(size_t i = 0; i < pl->numberOfPosts; ++i){
            std::cout << pl->posts[i]->loc << std::endl;
        }
        */
    }