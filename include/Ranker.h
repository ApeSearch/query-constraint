#pragma once

#ifndef _RANKER_H
#define _RANKER_H

#include "../libraries/AS/include/AS/string.h"
#include "../libraries/AS/include/AS/vector.h"
#include "../libraries/AS/include/AS/unique_ptr.h"
#include "Index.h"
#include "ISR.h"
#include "Tuple.h"
#include <vector>

class Ranker {
    public:

        //Dynamic weights : minToBeMost, minToBeNearTop

        Ranker(const IndexBlob* index, const APESEARCH::string& queryLine);

        APESEARCH::vector<RankedEntry> getTopTen();

    private:

        APESEARCH::unique_ptr<query::Tuple> buildParseTree(APESEARCH::string queryLine) {
            QueryParser query = QueryParser(queryLine);
            return APESEARCH::unique_ptr<query::Tuple>( query.FindOrConstraint());
        }

        double getScore(APESEARCH::vector<ISR*> &flattened, APESEARCH::vector<size_t> &indices, ISREndDoc* endDoc);
        double getURLScore(APESEARCH::vector<ISR*> &flattened, APESEARCH::vector<size_t> &indices, APESEARCH::string &url);
        double getAnchorScore(APESEARCH::vector<ISR*> &flattened, APESEARCH::vector<size_t> &indices, const IndexBlob* index, const size_t urlIndex);

        struct DynamicStats 
            {
            static constexpr double WordFrequentThreshold = 20;
            static constexpr double MaxToBeShort = 10;

            static constexpr double W_NumShortSpans = 2;
            static constexpr double W_NumInOrderSpans = 2;
            static constexpr double W_ExactPhrases = 6;
            static constexpr double W_NumSpansNearTop = 8;
            static constexpr double W_AllWordsFrequent = 5;
            static constexpr double W_MostWordsFrequent = 7;
            static constexpr double W_SomeWordsFrequent = 5;

            static constexpr double W_Anchor = 100;
            static constexpr double W_Url = 20;
            static constexpr double W_Title = 5;
            static constexpr double W_Body = 1;
            };

        struct StaticStats
            {
            static constexpr size_t MaxURLLengthToBeShort = 35;
            static constexpr size_t W_ShortURL = 5;
            };

        const IndexBlob* ib;


        APESEARCH::vector<APESEARCH::vector<ISR *>> flattened;
        APESEARCH::vector<APESEARCH::string> urls;

        APESEARCH::vector<RankedEntry> chunkResults;

        APESEARCH::unique_ptr<ISR> compiledTree;
        APESEARCH::unique_ptr<ISREndDoc> docEnd;
};


#endif