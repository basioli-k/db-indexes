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
    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db1");
    ver_table vtable("C:/Users/kbasi/git/db-indexes/examples/db1");

    std::vector<row> rows_hor, rows_ver;

    htable.read_rows(rows_hor, 3, 8);
    vtable.read_rows(rows_ver, 3, 8);

    print_rows(rows_hor);
    std::cout << "--------------\n";
    print_rows(rows_ver);

    return 0;
}