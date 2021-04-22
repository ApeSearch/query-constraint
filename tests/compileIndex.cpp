#include "../include/Builder.h"
#include <iostream>


int main () {
    Builder built = Builder("./processedFiles");

    std::cout << "Index Chunks Built!" << std::endl;

    return 0;
}