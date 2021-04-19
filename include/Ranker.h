#include "../libraries/AS/include/AS/string.h"
#include "../libraries/AS/include/AS/vector.h"
#include "ISR.h"

// static const struct DynamicStats 
//     {
//     static const size_t WordFrequentThreshold = 20;
//     static const size_t MaxToBeShort = 10;

//     static const size_t W_NumShortSpans = 5;
//     static const size_t W_NumInOrderSpans = 5;
//     static const size_t W_ExactPhrases = 15;
//     static const size_t W_NumSpansNearTop = 5;
//     static const size_t W_AllWordsFrequent = 5;
//     static const size_t W_MostWordsFrequent = 5;
//     static const size_t W_SomeWordsFrequent = 5;

//     static const size_t W_Anchor = 20;
//     static const size_t W_Url = 10;
//     static const size_t W_Title = 10;
//     static const size_t W_Body = 1;
//     };

// static const struct StaticStats
//     {
//     static const size_t ShortTitleThreshold = 20;
//     static const size_t MinNumAnchorText = 10;
//     static const size_t MaxURLLengthToBeShort = 20;

//     static const size_t W_Domain = 2;
//     static const size_t W_ShortTitle = 2;
//     static const size_t W_LotsAnchorText = 10;
//     static const size_t W_ShortURL = 2;
//     };



class Ranker {
    public:

        //Dynamic weights : minToBeMost, minToBeNearTop


        Ranker(ISR* tree, ISREndDoc* docEnd);
        double getRank(ISREndDoc const *docEnd, APESEARCH::string &url);

    private:
        APESEARCH::vector<ISR *> wordISRs;
        ISREndDoc* endDoc;
};