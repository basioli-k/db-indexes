#include <iostream>
#include <vector>
#include "headers/hor_table.h"
#include "headers/ver_table.h"
#include "headers/row.h"

void print_rows(std::vector<row>& rows) {
    for (auto& row : rows) {
        auto& row_ref = row.data();
        for (auto el : row_ref) {
            std::cout << el << "\n";
        }
        std::cout << "--------------\n";
    }
}

int main(int argc, char **argv) {
    // TODO make this command line argument
    // TODO update python scripts to get path for hor and path for ver
    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    ver_table vtable("C:/Users/kbasi/git/db-indexes/examples/db-ver");

    std::vector<row> rows_hor, rows_ver;


    htable.read_rows(rows_hor, 3, 8);

    htable.insert(rows_hor[0]);
    vtable.insert(rows_hor[0]);

    rows_hor.clear();
    htable.read_rows(rows_hor, 1, 12);
    vtable.read_rows(rows_ver, 1, 12);

    print_rows(rows_hor);
    std::cout << "--------------\n";
    print_rows(rows_ver);

    return 0;
}