#include "../include/Ranker.h"

Ranker::Ranker(ISR* tree, ISREndDoc* docEnd): endDoc(docEnd) {
    tree->Flatten(wordISRs);
}

double Ranker::getRank(ISREndDoc const *docEnd, APESEARCH::string &url){
    return 0;
}