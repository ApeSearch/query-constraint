#include "../include/Index.h"
#include <iostream>


IndexHT::IndexHT() : chunk(), urls(), docEndList(new DocEndPostingList), LocationsInIndex(0), MaximumLocation(0) {}

IndexHT::~IndexHT(){}


Location DocEndPostingList::appendToList(Location urlLoc, size_t urlIndex) {
    numOfDocs++;

    if(!posts.size()){
        posts.push_back(new EODPost(urlLoc, urlIndex));
        return 0;
    }
    else {
        Location curLoc = 0;
        for(Post* post : posts){
            curLoc += post->deltaPrev;
        }

        posts.push_back(new EODPost(urlLoc - curLoc, urlIndex));

        return curLoc;
    }
}

void WordPostingList::appendToList(Location tokenLoc, WordAttributes attribute) {
    
    if(!posts.size()){
        if(tokenLoc < k2Exp7)
            posts.emplace_back(tokenLoc);

        else if(tokenLoc < k2Exp14){
            posts.emplace_back((2 << 6) | (tokenLoc >> 8));
            posts.emplace_back(tokenLoc & 0xff);
        }

        else if(tokenLoc < k2Exp21){
            posts.emplace_back((6 << 5) | (tokenLoc >> 16));
            posts.emplace_back((tokenLoc >> 8) & 0xff);
            posts.emplace_back(tokenLoc & 0xff);
        }
        else{
            posts.emplace_back((0xe << 4) | (tokenLoc >> 24));
            posts.emplace_back((tokenLoc >> 16) & 0xff);
            posts.emplace_back((tokenLoc >> 8) & 0xff);
            posts.emplace_back(tokenLoc & 0xff);
        }

        posts.emplace_back(attribute);
    }

    else{
        Location curLoc = 0;
        

        posts.push_back(new WordPost(tokenLoc - curLoc, attribute));

        //std::cout << tokenLoc - curLoc << " " << posts[posts.size() - 1]->deltaPrev << std::endl;
    }

    numberOfPosts++;
}

void IndexHT::addDoc(APESEARCH::string url, APESEARCH::vector<APESEARCH::string> &bodyText, size_t endDocLoc){

    urls.push_back(url);

    //get Previous Document's Absolute Location to add to current word index
    Location prevDocLoc = docEndList.get()->appendToList(endDocLoc, urls.size() - 1);


    for(Location indexLoc = 0; indexLoc < bodyText.size(); ++indexLoc) {
        auto& word = bodyText[indexLoc];

        hash::Tuple<const char *, WordPostingList *> * entry = chunk.Find(word.cstr());
        if(!entry){
            WordPostingList * wl = new WordPostingList();
            entry = chunk.Find(word.cstr(), wl);
        }

        entry->value->appendToList(indexLoc + prevDocLoc, WordAttributeNormal);
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