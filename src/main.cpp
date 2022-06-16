#include <iostream>
#include <vector>
#include "headers/hor_table.h"

int main(int argc, char **argv) {
    // TODO make this command line argument
    hor_table table("C:/Users/kbasi/git/db-indexes/examples/db1");
    row r {1, 7, 3, 5};
    table.insert(r);

    return 0;
}