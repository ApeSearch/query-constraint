#include "../libraries/AS/include/AS/string.h"
#include "../libraries/AS/include/AS/vector.h"
#include "ISR.h"

class Ranker {
    public:
        Ranker(ISR* tree, ISREndDoc* docEnd);
        double getRank(ISREndDoc const *docEnd, APESEARCH::string &url);

    private:
        APESEARCH::vector<ISR *> wordISRs;
        ISREndDoc* endDoc;
};