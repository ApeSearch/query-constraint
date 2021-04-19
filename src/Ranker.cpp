#include "../include/Ranker.h"

Ranker::Ranker(ISR* tree, ISREndDoc* docEnd): endDoc(docEnd) {
    tree->Flatten(wordISRs);
}

double Ranker::getRank(ISREndDoc* docEnd, APESEARCH::string &url) {

    // for each ISR, see if there's a match in the 
    Location DocumentLength = docEnd->posts->curPost->loc - docEnd->posts->prevLoc;
    
    return 0;
}