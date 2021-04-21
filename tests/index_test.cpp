#include "queries.h"
#include "../include/IndexHT.h"
#include "../libraries/AS/include/AS/string.h"
#include <iostream>
#include <cassert>

int main() 
    {
        APESEARCH::vector<IndexEntry> words = {
            {"the", WordAttributeNormal, BodyText},
            {"cow", WordAttributeNormal, BodyText},
            {"the", WordAttributeNormal, BodyText},
            {"pig", WordAttributeNormal, BodyText},
            {"and", WordAttributeNormal, BodyText},
            {"all", WordAttributeNormal, BodyText},
            { "of", WordAttributeNormal, BodyText},
            {"the", WordAttributeNormal, BodyText},
            {"animals", WordAttributeNormal, BodyText},
        };

        IndexHT *index = new IndexHT();

        APESEARCH::vector<AnchorText> aText;
        index->addDoc("https://eecs440.com", words, aText, 9);
        index->addDoc("https://eecs441.com", words, aText, 9);

        APESEARCH::string strToFind = "the";

        hash::Tuple<APESEARCH::string, PostingList *> * entry = index->dict.Find(strToFind);
        hash::HashTable<APESEARCH::string, PostingList *>::iterator itr = index->dict.begin();

        for(size_t i = 0; i < entry->value->posts.size(); ++i) {
            std::cout << entry->value->posts[i]->loc << std::endl;
        }
        
        while(itr != index->dict.end()){
            std::cout << itr->key << std::endl;
            for(size_t i = 0; i < itr->value->posts.size(); ++i) {
                std::cout << itr->value->posts[i]->loc << " ";
            }
            std::cout << std::endl;
            itr++;
        }

        delete index;
    }