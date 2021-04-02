#include "../include/Index.h"
#include <iostream>

const APESEARCH::string the = "the";

IndexHT::IndexHT() : chunk(), urls(), docEndList(new DocEndPostingList), WordsInIndex(0), DocumentsInIndex(0), 
    LocationsInIndex(0), MaximumLocation(0) {}

IndexHT::~IndexHT(){}


void DocEndPostingList::appendToList(Location urlLoc, size_t urlIndex) {
    if(!posts.size())
        posts.push_back(new EODPost(urlLoc, urlIndex));
    else {
        Location curLoc = 0;
        for(Post* post : posts){
            curLoc += post->deltaPrev;
        }

        posts.push_back(new EODPost(urlLoc - curLoc, urlIndex));
    }
}

void WordPostingList::appendToList(Location tokenLoc, WordAttributes attribute) {
    if(!posts.size()) 
        posts.push_back(new WordPost(tokenLoc, attribute));
    else{
        Location curLoc = 0;
        for(Post* post : posts){
            curLoc += post->deltaPrev;
        }

        posts.push_back(new WordPost(tokenLoc - curLoc, attribute));

        //std::cout << tokenLoc - curLoc << " " << posts[posts.size() - 1]->deltaPrev << std::endl;
    }
}

void IndexHT::addDoc(APESEARCH::string url, APESEARCH::vector<APESEARCH::string> &bodyText, size_t endDocLoc){

    urls.push_back(url);
    docEndList.get()->appendToList(endDocLoc, urls.size() - 1);

    for(Location indexLoc = 0; indexLoc < bodyText.size(); ++indexLoc) {
        auto& word = bodyText[indexLoc];

        hash::Tuple<const char *, WordPostingList *> * entry = chunk.Find(word.cstr());

        if(!entry){
            WordPostingList * wl = new WordPostingList();
            entry = chunk.Find(word.cstr(), wl);
        }

        entry->value->appendToList(indexLoc, WordAttributeNormal);
    }
}

Post *PostingList::Seek(Location l) {
    Location curLoc = 0;

    for(Post* post : posts){
        curLoc += post->deltaPrev;

        if(curLoc == l)
            return post;
    }

    return nullptr;
}