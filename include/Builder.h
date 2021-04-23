#include "../libraries/AS/include/AS/vector.h"
#include "../libraries/AS/include/AS/string.h"

class Builder {
    public:
        // Given a directory of processed directories, build an in-memory index from it
        Builder(const char * processedDirectory);

    private:
        APESEARCH::vector<APESEARCH::string> processedFileNames;
};