#pragma once

#include "IndexHT.h"
#include "Index.h"

#include "../libraries/AS/include/AS/File.h"
#include "../libraries/AS/include/AS/string.h"
#include "../libraries/AS/include/AS/vector.h"
#include "../libraries/AS/include/AS/unique_mmap.h"



class IndexFileParser 
    {
        public:
            IndexFileParser(): currentChunk(0), index(new IndexHT()), entries(), aText() {}

            ~IndexFileParser() {}

            void writeFile(const char * fileName)
                {
                APESEARCH::File file = APESEARCH::File(fileName, O_RDONLY);
                parseFile(file);
                }
            
            //given a char buffer of beg and 1 past the end of the last number, returns the actual number
            size_t indexOffBuffer(const char * beg, const char * pastEnd) const{
                char buffer[32];

                strncpy(buffer, beg, pastEnd - beg);
                buffer[pastEnd - beg] = '\0';

                return static_cast<size_t>(atoi(buffer));
            }

            APESEARCH::unique_ptr<IndexHT> index;
            APESEARCH::vector<IndexEntry> entries;
            APESEARCH::vector<AnchorText> aText;
            APESEARCH::string url;

            int currentChunk;
            const Location MAX_LOCATION = 1000;

            private:
                char* parseBodyText(char * cur){
                    char* beg = cur;
                    while(*cur != '\n'){
                        while(*cur++ != ' ');

                        entries.push_back(IndexEntry{ APESEARCH::string(beg, 0, cur - beg - 1), WordAttributeNormal, BodyText});

                        while(*cur == ' ') //TODO: Supposed to get rid of just space characters
                            ++cur;
                        
                        beg = cur;
                    }

                    return cur + 1;
                }

                char* parseTitleIndicies(char * cur){
                    char* beg = cur;

                    while(*cur != '\n'){
                        while(*cur++ != ' ');

                        size_t index = indexOffBuffer(beg, cur - 1);

                        entries[index].plType = TitleText;
                        beg = cur;
                    }

                    return cur + 1;
                }

                char* parseAttributeIndicies(char * cur, WordAttributes attribute){
                    char* beg = cur;

                    while(*cur != '\n'){
                        while(*cur++ != ' ');

                        size_t index = indexOffBuffer(beg, cur - 1);

                        entries[index].attribute = attribute;
                        beg = cur;
                    }

                    return cur + 1;
                }

                char * parseAnchorText(char * cur){
                    while(*cur != '\0'){
                        while(*cur++ != '"'); //newline safeguard

                        char* beg = cur;
                        
                        while(*cur++ != '"');

                        APESEARCH::string text(beg, 0, cur - beg - 1);

                        assert(*cur++ == ' ');   

                        beg = cur;
                        while(*cur++ != '\n');

                        size_t freq = indexOffBuffer(beg, cur - 1);

                        aText.push_back(AnchorText{text, freq});
                    }

                    return cur;
                }


                void parseFile(APESEARCH::File &file) {
                    char* map = ( char * ) mmap( nullptr, file.fileSize(), PROT_READ, MAP_PRIVATE, file.getFD(), 0 );

                    char* end = map + file.fileSize();

                    char* beg = map;
                    char* cur = beg;

                    while(cur < end){
                        while(*cur++ != '\n' && cur < end);

                        url = APESEARCH::string(beg, 0, cur - beg - 1);

                        cur = parseBodyText(cur);
                        cur = parseTitleIndicies(cur);
                        cur = parseAttributeIndicies(cur, WordAttributeHeading);
                        cur = parseAttributeIndicies(cur, WordAttributeBold);

                        while(*cur++ != '\n'); //skip base

                        beg = cur;
                        while(*cur++ != '\n');
                        size_t numParagraphs = indexOffBuffer(beg, cur - 1);

                        beg = cur;
                        while(*cur++ != '\n');
                        size_t numHeadings = indexOffBuffer(beg, cur - 1);

                        beg = cur;
                        while(*cur++ != '\n');
                        size_t numSentences = indexOffBuffer(beg, cur - 1);

                        /**** Debugging Purposes ****
                        for(int i = 0; i < entries.size(); ++i){
                            APESEARCH::string attribute = "Bold";

                            if(entries[i].attribute == WordAttributeNormal)
                                attribute = "Normal";
                            else if(entries[i].attribute == WordAttributeHeading)
                                attribute = "Heading";

                            std::cout << entries[i].word << ' ' << entries[i].plType << ' ' << attribute << ' ' << entries[i].word.size() << std::endl;
                        }
                        */
                        cur = parseAnchorText(cur);

                        for(int i = 0; i < aText.size(); ++i){
                            std::cout << aText[i].text << ' ' << aText[i].freq << std::endl;
                        }

                        index->addDoc(url, entries, aText, entries.size());

                        if (index->MaximumLocation > MAX_LOCATION)
                            {
                            char buffer[64]; // The filename buffer.
                            snprintf(buffer, sizeof(char) * 32, "apechunk%i", currentChunk++);
                            // IndexFile()
                            index->dict.Optimize();
                            IndexFile hashFile( buffer, index.get() );
                            index = APESEARCH::unique_ptr<IndexHT>(new IndexHT());
                            }

                        entries = {};
                        aText = {};

                        assert(*cur++ == '\0');
                        beg = cur;
                    }

                    munmap( map, file.fileSize() );
                }

    };