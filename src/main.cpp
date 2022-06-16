#include <iostream>
#include <vector>
#include "headers/hor_table.h"

int main(int argc, char **argv) {
    hor_table table("C:/Users/kbasi/git/db-indexes/examples/db1");
    std::vector<int32_t> buff {1, 3, 5};
    table.read(buff, 4);
    
    return 0;
}