#include "../include/Ranker.h"
#include <math.h>
#include <chrono>

double Ranker::getScore(APESEARCH::vector<ISR*> &flattened, APESEARCH::vector<size_t> &indices, ISREndDoc* endDoc) {
    Location begLoc = endDoc->GetStartLocation();
    Location endLoc = endDoc->GetEndLocation();
    
    const size_t minToBeTop = (endLoc - begLoc) / 10;
    const size_t minToBeMost = indices.size()/2 + 1;

    APESEARCH::vector<size_t> freq(APESEARCH::vector<size_t>(indices.size(), 0));
    size_t numExactSpans = 0;
    size_t numShortSpans = 0;
    size_t numInOrderSpans = 0;
    size_t numSpansNearTop = 0;

    size_t furthestLoc = begLoc;

    bool calcAndReturn = false;
    while(furthestLoc < endLoc){
        //calculate furthest isr location
        for(size_t i = 0; i < indices.size(); ++i) {
            ISRWord * isr = (ISRWord * ) flattened[indices[i]];
            if(isr->GetStartLocation() > furthestLoc)
                furthestLoc = isr->GetStartLocation();
        }

        //Call Next, count freq, on each isr until they are past
        //the furthest loc, use prevLoc and curLoc to
        //determine span
        
        // if (flattened[indices[i]]->GetStartLocation() < endLoc) continue;
        APESEARCH::vector<Location> locations(indices.size());

        for(size_t i = 0; i < indices.size(); ++i) {
            ISRWord * isr = (ISRWord * ) flattened[indices[i]];

            //count freq until loc after furthest loc
            Post * nextPost = isr->Next(endDoc);
            while(nextPost && isr->GetStartLocation() <= furthestLoc){
                freq[i]++,    
                nextPost = isr->Next(endDoc);
            }

            //figure out doubles and triples
            if(!nextPost || isr->GetStartLocation() > endLoc){
                calcAndReturn = true;
                break;
                //maybe we still need to count doubles and triples?
            }

            assert(furthestLoc >= isr->posts->prevLoc);
            assert(isr->GetStartLocation() >= furthestLoc);
            //seeing if post before was closer or current one, maybe sanitize with abs()?
            if(furthestLoc - isr->posts->prevLoc < isr->GetStartLocation() - furthestLoc)
                locations[i] = isr->posts->prevLoc;
            else
                locations[i] = isr->GetStartLocation();
        }

        if(calcAndReturn)
            break;

        size_t startSpan = locations[0], endSpan = locations[0];
        size_t lastLoc = startSpan;
        bool inOrder = true;
        
        for(size_t i = 1; i < locations.size(); ++i){

            if(locations[i] > lastLoc)
                lastLoc = locations[i];
            else
                inOrder = false;

            if(startSpan > locations[i])
                startSpan = locations[i];
            
            if(endSpan < locations[i])
                endSpan = locations[i];
        }

        if(inOrder && endSpan - startSpan == indices.size() - 1)
            numExactSpans++;
        if(inOrder)
            numInOrderSpans++;
        if(endSpan - startSpan <= DynamicStats::MaxToBeShort)
            numShortSpans++;

        if(furthestLoc - begLoc <= minToBeTop)
            numSpansNearTop++;
    }

    size_t numFreq = 0;
    double overallScore = 0;

    for(size_t i = 0; i < freq.size(); ++i){
        if(freq[i] >= DynamicStats::WordFrequentThreshold)
            numFreq++;
    }

    if(numFreq == freq.size())
        overallScore += DynamicStats::W_AllWordsFrequent;
    else if(numFreq > minToBeMost)
        overallScore += DynamicStats::W_MostWordsFrequent;
    else if(numFreq > 1)
        overallScore += DynamicStats::W_SomeWordsFrequent;

    overallScore += DynamicStats::W_ExactPhrases * numExactSpans;
    overallScore += DynamicStats::W_NumInOrderSpans * numInOrderSpans;
    overallScore += DynamicStats::W_NumShortSpans * numShortSpans;
    overallScore += DynamicStats::W_NumSpansNearTop * numSpansNearTop;


    // std::cout << numExactSpans << std::endl;
    // std::cout << numInOrderSpans << std::endl;
    // std::cout << numShortSpans << std::endl;
    // std::cout << numSpansNearTop << std::endl;

    return overallScore;

}

double Ranker::getURLScore(APESEARCH::vector<ISR*> &flattened, APESEARCH::vector<size_t> &indices, APESEARCH::string &url)
    {
    int matches = 0;
    for (size_t index: indices)
        {
        size_t foundLocation = url.find(flattened[index]->GetNearestWord());
        if (foundLocation != APESEARCH::string::npos)
            ++matches;
        }

    return DynamicStats::W_Url * matches;
    }

double Ranker::getAnchorScore(APESEARCH::vector<ISR*> &flattened, APESEARCH::vector<size_t> &indices, const IndexBlob* index, const size_t urlIndex)
    {
    // build the anchor string and put a space between each term
    APESEARCH::string anchorString = "#";
    for (int i = 0; i < indices.size() - 1; ++i)
        {
        anchorString += flattened[indices[i]]->GetNearestWord();
        anchorString += " ";
        }
    // ensure that a space isn't added after the last word
    anchorString += flattened[indices.back()]->GetNearestWord();

    const SerializedAnchorText *foundAnchor = index->FindAnchorText(anchorString);

    AnchorListIterator itr(foundAnchor);
    return DynamicStats::W_Anchor * itr.FindUrlIndex(urlIndex);
    }


Ranker::Ranker(const IndexBlob* index, const APESEARCH::string& queryLine) : ib(index), chunkResults(), flattened() {
    APESEARCH::unique_ptr<query::Tuple> parseTree = buildParseTree(queryLine);
    if (!parseTree)
        return;

    compiledTree = APESEARCH::unique_ptr<ISR>(parseTree->Compile(ib));
    docEnd = APESEARCH::unique_ptr<ISREndDoc>(ib->getEndDocISR());

    compiledTree->FlattenStructure(flattened);

    urls = ib->getUrls();


    // for (auto entry : chunkResults)
    //     {
    //     std::cout << entry.url << " " << entry.rank << std::endl;
    //     }
}

APESEARCH::vector<RankedEntry> Ranker::getTopTen() {
    // Gets the first post Returns seek past 0
    const std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();

    size_t documentIndex = 0;
    
    Post* post = compiledTree->NextDocument(docEnd.get());
    if(!post)
        return {};
    

    while(post) {
        bool filesToSearch = docEnd->Seek(post->loc, docEnd.get());
        if (!filesToSearch)
            return chunkResults;
        
        documentIndex = docEnd->posts->curPost->tData;
        Location startLoc = docEnd->GetStartLocation();

        double titleScore = 0; 
        double bodyScore = 0;
        double URLScore = 0;
        double anchorScore = 0;

        for (APESEARCH::vector<ISR*>& orTerms : flattened) {
            APESEARCH::vector<size_t> titleIndices;
            APESEARCH::vector<size_t> bodyIndices;
            // for (auto term : orTerms) {
            //     std::cout << term->GetNearestWord() << " ";
            // }
            // std::cout << endl;
            for(size_t i = 0; i < orTerms.size(); ++i)
                {
                APESEARCH::string word = orTerms[i]->GetNearestWord();

                orTerms[i]->Seek(startLoc, docEnd.get());

                if(word[0] == '$')
                    titleIndices.push_back(i);
                else
                    bodyIndices.push_back(i);
                }

            if (titleIndices.size()) {
                titleScore += DynamicStats::W_Title * getScore(orTerms, titleIndices, docEnd.get());
            }
            if (bodyIndices.size()) {
                bodyScore += DynamicStats::W_Body * getScore(orTerms, bodyIndices, docEnd.get());
                
                if(bodyIndices.size() == 1)
                    bodyScore = log(bodyScore);

                URLScore += getURLScore(orTerms, bodyIndices, urls[documentIndex].convertToLower());
                anchorScore += getAnchorScore(orTerms, bodyIndices, ib, documentIndex);
            }
        }

        double rank = titleScore + bodyScore + URLScore + anchorScore;

        if( urls[documentIndex].size() <= StaticStats::MaxURLLengthToBeShort )
            rank += StaticStats::W_ShortURL;


        size_t foundLocation = urls[documentIndex].find(APESEARCH::string("tumblr"));

        if(foundLocation != APESEARCH::string::npos)
            rank = log(rank);
        
        if (chunkResults.size() < 10)
            {
            chunkResults.push_back(RankedEntry(urls[documentIndex], rank));
            }
        else
            {
            double minRank = chunkResults[0].rank;
            size_t minIndex = 0;
            RankedEntry newEntry = RankedEntry(urls[documentIndex], rank);
            for(int i = 1; i < chunkResults.size(); ++i)
                {
                if(chunkResults[i].rank < minRank)
                    minRank = chunkResults[i].rank, minIndex = i;
                }

            APESEARCH::swap( newEntry, chunkResults[minIndex]);
            }

        const auto end = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > 20000)
            return chunkResults;


        post = compiledTree->NextDocument(docEnd.get());
    }

    // std::cout << "========" << std::endl;
    // for (auto results : chunkResults) {
    //     std::cout << results.rank << ' ' << results.url << std::endl;
    // }
    // std::cout << "========" << std::endl;
    return chunkResults;
}
