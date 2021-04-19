#include "../libraries/AS/include/AS/string.h"
#include "../libraries/AS/include/AS/vector.h"
#include "ISR.h"

static const struct DynamicStats 
    {
    static const size_t WordFrequentThreshold = 20;
    static const size_t MaxToBeShort = 10;

    static const size_t W_NumShortSpans = 5;
    static const size_t W_NumInOrderSpans = 5;
    static const size_t W_ExactPhrases = 15;
    static const size_t W_NumSpansNearTop = 5;
    static const size_t W_AllWordsFrequent = 5;
    static const size_t W_MostWordsFrequent = 5;
    static const size_t W_SomeWordsFrequent = 5;
    }



class Ranker {
    public:

        //Dynamic weights : minToBeMost, minToBeNearTop


        Ranker(ISR* tree, ISREndDoc* docEnd);
        double getRank(ISREndDoc const *docEnd, APESEARCH::string &url);

    private:
        APESEARCH::vector<ISR *> wordISRs;
        ISREndDoc* endDoc;
};