#include "queries.h"
#include "../include/Index.h"
#include "../libraries/AS/include/AS/string.h"
#include <iostream>
#include <cassert>

int main() 
    {
        APESEARCH::vector<APESEARCH::string> words = {
            APESEARCH::string("the"),
            APESEARCH::string("cow"),
            APESEARCH::string("the"),
            APESEARCH::string("pig"),
            APESEARCH::string("and"),
            APESEARCH::string("all"),
            APESEARCH::string("of"),
            APESEARCH::string("the"),
            APESEARCH::string("animals"),
        };

        
        IndexHT *index = new IndexHT();
        index->addDoc("https://eecs440.com", words, 9, BodyText);
        index->addDoc("https://eecs441.com", words, 19, AnchorText); 

        APESEARCH::string strToFind = "$the";

        hash::Tuple<APESEARCH::string, PostingList *> * entry = index->dict.Find(strToFind);
        hash::HashTable<APESEARCH::string, PostingList *>::Iterator itr = index->dict.begin();

        for(size_t i = 0; i < entry->value->posts.size(); ++i){
            std::cout << entry->value->posts[i]->loc << std::endl;
        }
        
        while(itr != index->dict.end()){
            std::cout << itr->key << std::endl;
            itr++;
        }

        delete index;

    }