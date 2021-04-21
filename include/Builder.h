#include "../libraries/AS/include/AS/vector.h"
#include "../libraries/AS/include/AS/string.h"

class Builder {
    public:
        Builder(const char * processedDirectory);

    private:
        APESEARCH::vector<APESEARCH::string> processedFileNames;
};