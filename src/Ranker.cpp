#include "../include/Ranker.h"

double Ranker::getScore(APESEARCH::vector<ISR*> &flattened, APESEARCH::vector<size_t> &indices, ISREndDoc* endDoc){

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
        bool pastEnd = false;
        APESEARCH::vector<Location> locations(indices.size());

        for(size_t i = 0; i < indices.size(); ++i) {
            ISRWord * isr = (ISRWord * ) flattened[indices[i]];

            //count freq until loc after furthest loc
            Post * nextPost = isr->Next(endDoc);
            while(nextPost && isr->GetStartLocation() <= furthestLoc){
                freq[i]++,    
                nextPost = isr->Next(endDoc);
            }

            if(!nextPost || isr->GetStartLocation() > endLoc){
                pastEnd = true;
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

        if(pastEnd)
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


    std::cout << numExactSpans << std::endl;
    std::cout << numInOrderSpans << std::endl;
    std::cout << numShortSpans << std::endl;
    std::cout << numSpansNearTop << std::endl;

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



Ranker::Ranker(const IndexBlob* index, const APESEARCH::string queryLine) : ib(index), chunkResults(), flattened(){
    APESEARCH::unique_ptr<query::Tuple> parseTree = buildParseTree(queryLine);
    compiledTree = APESEARCH::unique_ptr<ISR>(parseTree->Compile(ib));
    docEnd = APESEARCH::unique_ptr<ISREndDoc>(ib->getEndDocISR());
    compiledTree->Flatten(flattened);

    urls = ib->getUrls();
    // Gets the first post Returns seek past 0
    Post *post = compiledTree->NextDocument(docEnd.get()); 

    size_t documentIndex = 0;
    //need
    while(post){
        bool filesToSearch = docEnd->Seek(post->loc, docEnd.get());
            if (!filesToSearch)
                return;
        
        documentIndex = docEnd->posts->curPost->tData;
        Location startLoc = docEnd->GetStartLocation();

        APESEARCH::vector<size_t> titleIndices;
        APESEARCH::vector<size_t> bodyIndices;

        for(size_t i = 0; i < flattened.size(); ++i){
            APESEARCH::string word = ((ISRWord *) flattened[i])->word;

            flattened[i]->Seek(startLoc, docEnd.get());

            if(word[0] == '$')
                titleIndices.push_back(i);
            else
                bodyIndices.push_back(i);
        }

        //double titleScore = DynamicStats::W_Title * getScore(flattened, titleIndices, docEnd.get());
        double bodyScore = DynamicStats::W_Body * getScore(flattened, bodyIndices, docEnd.get());

        double URLScore = getURLScore(flattened, bodyIndices, urls[documentIndex]);

        std::cout << ' ' << bodyScore << ' ' << urls[documentIndex] << std::endl;
        //URL score
        //AnchorText score
        //insertion sort

        post = compiledTree->NextDocument(docEnd.get());

    }
}
