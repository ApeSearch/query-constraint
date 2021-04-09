#include "../include/Index.h"
#include "../include/ISR.h"
#include <iostream>


IndexHT::IndexHT() : dict(), urls(), LocationsInIndex(0), MaximumLocation(0) {}

IndexHT::~IndexHT(){
    hash::HashTable<APESEARCH::string, PostingList *>::Iterator itr = dict.begin();

    while(itr != dict.end()){
        delete itr->value;
        itr++;
    }
}


void DocEndPostingList::appendToList(Location loc_, Attributes attribute, size_t lastDocIndex){
    if(posts.size())
        loc_ += posts.back()->loc;
    posts.push_back(new EODPost(loc_, attribute.urlIndex));
    numOfDocs++;
}

void WordPostingList::appendToList(Location loc_, Attributes attribute, size_t lastDocIndex){
    loc_ += lastDocIndex;
    posts.push_back(new WordPost(loc_, attribute.attribute));
    numberOfPosts++;
}

void IndexHT::addDoc(APESEARCH::string url, APESEARCH::vector<APESEARCH::string> text, 
    size_t endDocLoc, PostingListType type){
    
    urls.push_back(url);


    hash::Tuple<APESEARCH::string, PostingList *> * entry = dict.Find(APESEARCH::string("%"));
    size_t lastDocIndex = 0;

    if(entry)
        lastDocIndex = entry->value->posts.back()->loc;
    else
        entry = dict.Find(APESEARCH::string("%"), new DocEndPostingList());

    entry->value->appendToList(endDocLoc, urls.size() - 1);

    for(Location indexLoc = 0; indexLoc < text.size(); ++indexLoc) {
        APESEARCH::string word = text[indexLoc];
        switch (type) {
            case TitleText:
                word.push_front('%');
                break;
            case URL:
                word.push_front('#');
                break;
            case AnchorText:
                word.push_front('$');
                break;
            default:
                break;
        }
        entry = dict.Find(word);

        if(!entry)
            entry = dict.Find(word, new WordPostingList());

        entry->value->appendToList(indexLoc, WordAttributeNormal, lastDocIndex);
    }
}

Post *PostingList::Seek(Location l) {
    Location index = 0;
    for(; index < l && index < posts.size(); ++index);

    if(index == posts.size())
        return nullptr;

    return posts[index];
}

ISRWord* IndexHT::getWordISR ( APESEARCH::string word ) {
    hash::Tuple<APESEARCH::string, PostingList *> * entry = dict.Find(word);
    return new ISRWord(entry->value);
}

ISREndDoc* IndexHT::getEndDocISR ( ) {
    hash::Tuple<APESEARCH::string, PostingList *> * entry = dict.Find(APESEARCH::string("%"));
    return new ISREndDoc(entry->value);
}


/*
Below is code that will be used for writing the index into the disk.

1. Variable Length Encoding of Deltas 

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

*/
