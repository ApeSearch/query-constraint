#include "../include/Builder.h"
#include "../libraries/AS/include/AS/listdir.h"
#include "../include/IndexFileParser.h"

#include <iostream>

Builder::Builder(const char * processedDirectory) : processedFileNames(listdir(processedDirectory)) 
    {
    IndexFileParser parser;
    for (APESEARCH::string filename : processedFileNames)
        {
        std::cout << "Parsing Chunk Name : " << filename << std::endl;
        parser.writeFile(filename.cstr());
        }
    }