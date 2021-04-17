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
            IndexFileParser(const char * fileName): file(fileName, O_RDONLY), index(new IndexHT()){
                parseFile();
            }

            ~IndexFileParser(){
                delete index;
            }
            
            //given a char buffer of beg and 1 past the end of the last number, returns the actual number
            size_t indexOffBuffer(const char * beg, const char * pastEnd) const{
                char buffer[32];

                strncpy(buffer, beg, pastEnd - beg);
                buffer[pastEnd - beg] = '\0';

                return static_cast<size_t>(atoi(buffer));
            }

            IndexHT* index;
            APESEARCH::File file;
            APESEARCH::vector<IndexEntry> entries;
            APESEARCH::string url;


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

                        char buffer[32];

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

                        char buffer[32];

                        size_t index = indexOffBuffer(beg, cur - 1);

                        entries[index].attribute = attribute;
                        beg = cur;
                    }

                    return cur + 1;
                }


                void parseFile() {
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

                        size_t dot1 = url.find('.');
                        size_t dot2 = url.find('.', dot1 + 1);
                        APESEARCH::string urlString = APESEARCH::string(url, dot1 + 1, dot2 - dot1 - 1);
                        entries.push_back(IndexEntry{urlString, WordAttributeNormal, URL});

                        index->addDoc(url, entries, entries.size());
                        APESEARCH::vector<IndexEntry> temp;
                        entries = temp;
                        assert(*cur++ == '\0');
                    }

                    munmap( map, file.fileSize() );
                }

    };