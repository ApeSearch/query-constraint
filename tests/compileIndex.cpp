#include "../include/Builder.h"
#include <iostream>


int main () {
    Builder built = Builder("./condensed");

    std::cout << "Index Chunks Built!" << std::endl;

    return 0;
}