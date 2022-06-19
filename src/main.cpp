#include <iostream>
#include <vector>
#include "headers/hor_table.h"
#include "headers/row.h"

int main(int argc, char **argv) {
    // TODO make this command line argument
    hor_table table("C:/Users/kbasi/git/db-indexes/examples/db1");
    std::vector<row> rows;

    for (auto& row : rows) {
        auto& row_ref = row.data();
        for (auto el : row_ref) {
            std::cout << el << "\n";
        }
        std::cout << "--------------\n";
    }

    return 0;
}