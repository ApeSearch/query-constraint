#pragma once

#include "IndexHT.h"
#include "../libraries/AS/include/AS/File.h"
#include "../libraries/AS/include/AS/string.h"
#include "../libraries/AS/include/AS/vector.h"
#include "../libraries/AS/include/AS/unique_mmap.h"



class IndexFileParser 
    {
        public:
            IndexFileParser(){}
            IndexFileParser(const char * fileName): file(fileName, O_RDONLY){
                parseFile();
            }
            
            //given a char buffer of beg and 1 past the end of the last number, returns the actual number
            size_t indexOffBuffer(const char * beg, const char * pastEnd){
                char buffer[32];

                strncpy(buffer, beg, pastEnd - beg);
                buffer[pastEnd - beg] = '\0';

                return static_cast<size_t>(atoi(buffer));
            }


            APESEARCH::File file;
            APESEARCH::vector<APESEARCH::vector<IndexEntry> > entries;
            APESEARCH::vector<APESEARCH::string> urls;


            private:
                char* parseBodyText(char * cur, size_t curDoc){
                    char* beg = cur;
                    while(*cur != '\n'){
                        while(*cur++ != ' ');

                        entries[curDoc].push_back(IndexEntry{ APESEARCH::string(beg, 0, cur - beg - 1), WordAttributeNormal, BodyText});

                        while(*cur == ' ') //TODO: Supposed to get rid of just space characters, doesn't work
                            ++cur;
                        
                        beg = cur;
                    }

                    return cur + 1;
                }

                char* parseTitleIndicies(char * cur, size_t curDoc){
                    char* beg = cur;

                    while(*cur != '\n'){
                        while(*cur++ != ' ');

                        char buffer[32];

                        size_t index = indexOffBuffer(beg, cur - 1);

                        entries[curDoc][index].plType = TitleText;
                        beg = cur;
                    }

                    return cur + 1;
                }

                char* parseAttributeIndicies(char * cur, size_t curDoc, WordAttributes attribute){
                    char* beg = cur;

                    while(*cur != '\n'){
                        while(*cur++ != ' ');

                        char buffer[32];

                        size_t index = indexOffBuffer(beg, cur - 1);

                        entries[curDoc][index].attribute = attribute;
                        beg = cur;
                    }

                    return cur + 1;
                }


                void parseFile() {
                    char* map = ( char * ) mmap( nullptr, file.fileSize(), PROT_READ, MAP_PRIVATE, file.getFD(), 0 );

                    char* end = map + file.fileSize();

                    char* beg = map;
                    char* cur = beg;

                    size_t curDoc = 0;
                    while(cur < end){
                        entries.push_back(APESEARCH::vector<IndexEntry>());
                        while(*cur++ != '\n' && cur < end);

                        APESEARCH::string url = APESEARCH::string(beg, 0, cur - beg - 1);
                        urls.push_back(url);

                        size_t dot1 = url.find('.');
                        size_t dot2 = url.find('.', dot1 + 1);

                        APESEARCH::string urlString = APESEARCH::string(url, dot1 + 1, dot2 - dot1 - 1);
                        entries[curDoc].push_back( IndexEntry{urlString, WordAttributeNormal, URL});

                        cur = parseBodyText(cur, curDoc);
                        cur = parseTitleIndicies(cur, curDoc);
                        cur = parseAttributeIndicies(cur, curDoc, WordAttributeHeading);
                        cur = parseAttributeIndicies(cur, curDoc, WordAttributeBold);

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

                        std::cout << numParagraphs << ' ' << numHeadings << ' ' << numSentences << std::endl;

                        ++curDoc;
                        assert(*cur++ == '\0');
                    }

                    munmap( map, file.fileSize() );
                }

    };