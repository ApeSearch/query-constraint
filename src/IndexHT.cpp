#include "../include/IndexHT.h"
#include "../include/ISR.h"
#include "../libraries/AS/include/AS/delta.h"
#include "../include/Index.h"
#include <iostream>


IndexHT::IndexHT() : dict(), urls(), LocationsInIndex(0), MaximumLocation(0), numDocs(0), calcBytes(0), bytes(0){}

IndexHT::~IndexHT(){
    auto vecOfBuckets = dict.vectorOfBuckets();

    for(size_t i = 0; i < vecOfBuckets.size(); ++i){
        for(size_t sameChain = 0; sameChain < vecOfBuckets[i].size(); ++sameChain){
            delete vecOfBuckets[i][sameChain]->tuple.value;
        }
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

void AnchorPostingList::appendToList(Location freq, size_t urlIndex, size_t lastDocIndex){
    posts.push_back(new AnchorPost(freq, urlIndex));
}

uint32_t WordPostingList::bytesRequired(const APESEARCH::string &key) {

    if(calcBytes)
        return bytesList;
    uint32_t numBytes = key.size() + 1;
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

        //encode attributes
        //deltas.push_back(static_cast<uint8_t>(wp->tData));
    }

    numBytes += deltas.size();
    calcBytes = true;
    bytesList = numBytes;

    return numBytes;
}


uint32_t DocEndPostingList::bytesRequired(const APESEARCH::string &key) { //implement

    if(calcBytes)
        return bytesList;

    uint32_t numBytes = key.size() + 1;
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
    calcBytes = true;
    bytesList = numBytes;

    return numBytes;
}

uint32_t AnchorPostingList::bytesRequired( const APESEARCH::string &key ) {
    if(calcBytes)
        return bytesList;

    //std::cout << "BR: " << key << std::endl;

    uint32_t numBytes = key.size() + 1;
    numBytes += sizeof( SerializedAnchorText );

    for(size_t i = 0; i < posts.size(); ++i) {
        AnchorPost* aPost = (AnchorPost * ) posts[i];

        APESEARCH::vector<uint8_t> bytes = encodeDelta(aPost->loc); //loc refers to freq

        for(size_t byte = 0; byte < bytes.size(); ++byte)
            deltas.push_back(bytes[byte]);
        
        bytes = encodeDelta(aPost->tData);

        for(size_t byte = 0; byte < bytes.size(); ++byte)
            deltas.push_back(bytes[byte]);
    }

    numBytes += deltas.size();
    calcBytes = true;
    bytesList = numBytes;

    return numBytes;
}

void IndexHT::addDoc(APESEARCH::string url, const APESEARCH::vector<IndexEntry> &text, const APESEARCH::vector<AnchorText> &aText,
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
    
    MaximumLocation = lastDocIndex + endDocLoc; //Keep Track of Maximum Location, location of end of last doc
    LocationsInIndex = text.size() + 1; //Add 1 for end doc location + number of tokens in doc
    numDocs++; //Keeps track of the number of documents

    DocEndPostingList * docEndList = (DocEndPostingList *) entry->value;

    docEndList->appendToList(endDocLoc, urls.size() - 1, lastDocIndex);

    for(Location indexLoc = 0; indexLoc < text.size(); ++indexLoc) {
        APESEARCH::string word = text[indexLoc].word;
        if(text[indexLoc].plType == TitleText)
            word.push_front('$');

        entry = dict.Find(word);


        //Different actions for anchorText vs regularText

        if(!entry) //add AnchorPostingList if is anchor text
            entry = dict.Find(word, new WordPostingList());

        WordPostingList * wordList = (WordPostingList *) entry->value;
        wordList->appendToList(indexLoc, static_cast<size_t>(text[indexLoc].attribute), lastDocIndex);
    }

    for(size_t anchor = 0; anchor < aText.size(); ++anchor) {
        APESEARCH::string text = aText[anchor].text;
        text.push_front('#');

        entry = dict.Find(text);

        if(!entry)
            entry = dict.Find(text, new AnchorPostingList());
        
        AnchorPostingList * anchorList = (AnchorPostingList * ) entry->value;
        anchorList->appendToList(aText[anchor].freq, urls.size() - 1);
    }
    
}

Post *PostingList::Seek(Location l) {
    Location index = 0;
    for(; index < posts.size() && (l > posts[index]->loc && posts[index]->loc != l); ++index);

    if(index == posts.size())
        return nullptr;

    return posts[index];
}

uint32_t IndexHT::BytesRequired() {

    if(calcBytes)
        return bytes;

    uint32_t bytesRequired = sizeof( IndexBlob );
    bytesRequired += sizeof( uint32_t ) * dict.table_size( );

    APESEARCH::vector< APESEARCH::vector< hash::Bucket< APESEARCH::string, PostingList*> *> > vec = dict.vectorOfBuckets();
    std::cout << "BytesRequired" << std::endl;
    for(size_t index = 0; index < vec.size(); ++index){
        for(size_t sameChain = 0; sameChain < vec[index].size(); ++sameChain){
            
            hash::Bucket<APESEARCH::string, PostingList*> * bucket = vec[index][sameChain];
            bytesRequired += bucket->tuple.value->bytesRequired(bucket->tuple.key);
        }
        bytesRequired += sizeof( uint32_t ); // Signifies end of the chained posting lists...
    }

    // Bytes for the url vector
    for(size_t i = 0; i < urls.size(); ++i)
        bytesRequired += urls[i].size() + 1;

    calcBytes = true;
    bytes = bytesRequired;
    return bytesRequired;
}


