#include "../include/Builder.h"
#include <iostream>


int main () {
    Builder built = Builder("../crawler/condensedFiles");

    std::cout << "Index Chunks Built!" << std::endl;

    return 0;
}
