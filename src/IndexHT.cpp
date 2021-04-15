#include "../include/IndexHT.h"
#include "../include/ISR.h"
#include "../libraries/AS/include/AS/delta.h"
#include "../include/Index.h"
#include <iostream>


IndexHT::IndexHT() : dict(), urls(), LocationsInIndex(0), MaximumLocation(0), numDocs(0) {}

IndexHT::~IndexHT(){
    hash::HashTable<APESEARCH::string, PostingList *>::iterator itr = dict.begin();

    while(itr != dict.end()) {
        delete itr->value;
        itr++;
    }
}

void DocEndPostingList::appendToList(Location loc_, size_t urlIndex, size_t lastDocIndex){
    if(posts.size())
        loc_ += lastDocIndex;
    posts.push_back(new EODPost(loc_, urlIndex));

}

void WordPostingList::appendToList(Location loc_, size_t attribute, size_t lastDocIndex){
    loc_ += lastDocIndex;
    posts.push_back(new WordPost(loc_, attribute));
}

size_t WordPostingList::bytesRequired(const APESEARCH::string &key) {

    size_t numBytes = key.size() + 1;
    Location absoluteLocation = 0;
    numBytes += sizeof( SerializedPostingList );


    for(size_t i = 0; i < posts.size(); ++i) {
        WordPost* wp = (WordPost * ) posts[i];

        Location temp = absoluteLocation;
        absoluteLocation = wp->loc;
        APESEARCH::vector<uint8_t> bytes = encodeDelta(wp->loc - temp);

        for(size_t byte = 0; byte < bytes.size(); ++byte)
            deltas.push_back(bytes[byte]);

        //std::cout << decodeDelta(bytes) << std::endl;

        deltas.push_back(static_cast<uint8_t>(wp->tData));
    }

    numBytes += deltas.size();

    bytesList = numBytes;

    return numBytes;
}


size_t DocEndPostingList::bytesRequired(const APESEARCH::string &key) { //implement
    size_t numBytes = key.size() + 1;
    Location absoluteLocation = 0;
    numBytes += sizeof( SerializedPostingList );

    for(size_t i = 0; i < posts.size(); ++i) {
        EODPost* docEndp = (EODPost * ) posts[i];

        Location temp = absoluteLocation;
        absoluteLocation = docEndp->loc;
        APESEARCH::vector<uint8_t> bytes = encodeDelta(docEndp->loc - temp);

        for(size_t byte = 0; byte < bytes.size(); ++byte)
            deltas.push_back(bytes[byte]);
        
        bytes = encodeDelta(docEndp->tData);

        for(size_t byte = 0; byte < bytes.size(); ++byte)
            deltas.push_back(bytes[byte]);
    }

    numBytes += deltas.size();
    bytesList = numBytes;

    return numBytes;
} 

void IndexHT::addDoc(APESEARCH::string url, APESEARCH::vector<IndexEntry> &text, 
    size_t endDocLoc){ //change
    
    urls.push_back(url);

    hash::Tuple<APESEARCH::string, PostingList *> * entry = dict.Find(APESEARCH::string("%"));

    //stores absolute location of last doc
    //so current tokens can add it to their
    //relative location for determining absolute
    //location in regards to a whole index chunk
    size_t lastDocIndex = 0;
    
    if(entry)
        lastDocIndex = entry->value->posts.back()->loc + 1; 
    else
        entry = dict.Find(APESEARCH::string("%"), new DocEndPostingList());
    
    MaximumLocation = endDocLoc; //Keep Track of Maximum Location, location of end of last doc
    LocationsInIndex = text.size() + 1; //Add 1 for end doc location + number of tokens in doc
    numDocs++; //Keeps track of the number of documents

    DocEndPostingList * docEndList = (DocEndPostingList *) entry->value;

    docEndList->appendToList(endDocLoc, url.size() - 1, lastDocIndex);

    for(Location indexLoc = 0; indexLoc < text.size(); ++indexLoc) {
        APESEARCH::string word = text[indexLoc].word;
        switch (text[indexLoc].plType) {
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

        WordPostingList * wordList = (WordPostingList *) entry->value;
        wordList->appendToList(indexLoc, static_cast<size_t>(text[indexLoc].attribute), lastDocIndex);
    }
}

Post *PostingList::Seek(Location l) {
    Location index = 0;
    for(; index < posts.size() && (l > posts[index]->loc && posts[index]->loc != l); ++index);

    if(index == posts.size())
        return nullptr;

    return posts[index];
}

ISRWord* IndexHT::getWordISR ( APESEARCH::string word ) {
    hash::Tuple<APESEARCH::string, PostingList *> * entry = dict.Find(word);
    return entry ? new ISRWord(entry->value, this, word) : nullptr;
}

ISREndDoc* IndexHT::getEndDocISR ( ) {
    hash::Tuple<APESEARCH::string, PostingList *> * entry = dict.Find(APESEARCH::string("%"));
    return new ISREndDoc(entry->value, this);
}

size_t IndexHT::BytesRequired() {
    size_t bytesRequired = sizeof( IndexBlob );
    bytesRequired += sizeof( size_t ) * dict.table_size( );

    APESEARCH::vector< APESEARCH::vector< hash::Bucket< APESEARCH::string, PostingList*> *> > vec = dict.vectorOfBuckets();

    for(size_t index = 0; index < vec.size(); ++index){
        for(size_t sameChain = 0; sameChain < vec[index].size(); ++sameChain){
            hash::Bucket<APESEARCH::string, PostingList*> * bucket = vec[index][sameChain];
            bytesRequired += bucket->tuple.value->bytesRequired(bucket->tuple.key);
        }
        bytesRequired += sizeof( size_t ); // Signifies end of the chained posting lists...
    }

    // Bytes for the url vector
    for(size_t i = 0; i < urls.size(); ++i)
        bytesRequired += urls[i].size() + 1;


    return bytesRequired;
}


