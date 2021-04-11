#include "../include/IndexHT.h"
#include "../include/ISR.h"
#include "../libraries/AS/include/AS/delta.h"
#include <iostream>


IndexHT::IndexHT() : dict(), urls(), LocationsInIndex(0), MaximumLocation(0), numDocs(0) {}

IndexHT::~IndexHT(){
    hash::HashTable<APESEARCH::string, PostingList *>::iterator itr = dict.begin();

    while(itr != dict.end()) {
        delete itr->value;
        itr++;
    }
}

void DocEndPostingList::appendToList(Location loc_, Attributes attribute, size_t lastDocIndex){
    if(posts.size())
        loc_ += posts.back()->loc;
    posts.push_back(new EODPost(loc_, attribute.urlIndex));

}

void WordPostingList::appendToList(Location loc_, Attributes attribute, size_t lastDocIndex){
    loc_ += lastDocIndex;
    posts.push_back(new WordPost(loc_, attribute.attribute));
}

size_t WordPostingList::bytesRequired() {
    size_t numBytes = 0;
    Location absoluteLocation = 0;
    numBytes += sizeof( SerializedPostingList );


    for(size_t i = 0; i < posts.size(); ++i) {
        Location temp = absoluteLocation;
        absoluteLocation = posts[i]->loc;
        APESEARCH::vector<uint8_t> bytes = encodeDelta(posts[i]->loc - temp);

        numBytes += bytes.size() + 1;

        for(size_t byte = 0; byte < bytes.size(); ++byte)
            deltas.push_back(bytes[byte]);
        
        deltas.push_back(posts[i]->attribute.attribute);
    }

    return numBytes;
}


size_t DocEndPostingList::bytesRequired() {
    
} 



void IndexHT::addDoc(APESEARCH::string url, APESEARCH::vector<APESEARCH::string> &text, 
    size_t endDocLoc, PostingListType type){
    
    urls.push_back(url);


    hash::Tuple<APESEARCH::string, PostingList *> * entry = dict.Find(APESEARCH::string("%"));
    size_t lastDocIndex = 0;

    if(entry)
        lastDocIndex = entry->value->posts.back()->loc;
    else
        entry = dict.Find(APESEARCH::string("%"), new DocEndPostingList());
    
    MaximumLocation = endDocLoc; //Keep Track of Maximum Location, location of end of last doc
    LocationsInIndex = text.size() + 1; //Add 1 for end doc location + number of tokens in doc
    numDocs++;

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
    for(; index < posts.size() && posts[index]->loc != l; ++index);

    if(index == posts.size())
        return nullptr;

    return posts[index];
}

ISRWord* IndexHT::getWordISR ( APESEARCH::string word ) {
    hash::Tuple<APESEARCH::string, PostingList *> * entry = dict.Find(word);
    return new ISRWord(entry->value, this);
}

ISREndDoc* IndexHT::getEndDocISR ( ) {
    hash::Tuple<APESEARCH::string, PostingList *> * entry = dict.Find(APESEARCH::string("%"));
    return new ISREndDoc(entry->value, this);
}

void IndexHT::SerializeIndex(const APESEARCH::string &fileName) {
    size_t bytesRequired = sizeof( IndexBlob );

    APESEARCH::vector< APESEARCH::vector< hash::Bucket< APESEARCH::string, PostingList*> *> > vec = dict.vectorOfBuckets();

    for(size_t index = 0; index < vec.size(); ++index){
        for(size_t sameChain = 0; sameChain < vec[index].size(); ++sameChain){
            hash::Bucket<APESEARCH::string, PostingList*> * bucket = vec[index][sameChain];
            bytesRequired += bucket->tuple.value->bytesRequired();
        }
        bytesRequired += sizeof( size_t ); // Signifies end of the chained posting lists...
    }
    // Bytes for the url vector
    for(size_t i = 0; i < urls.size(); ++i)
        bytesRequired += urls[i].size() + 1;
    

    void *buffer;
    IndexBlob * ptr = reinterpret_cast< IndexBlob * >( buffer );
    ptr->MaxAbsolLoc = MaximumLocation;
}


