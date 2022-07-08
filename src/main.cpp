#include <iostream>
#include <vector>
#include "headers/hor_table.h"
#include "headers/ver_table.h"
#include "headers/row.h"
#include "headers/stopwatch.h"
#include "headers/io_handler.h"

void print_rows(std::vector<row>& rows) {
    for (auto& row : rows) {
        auto& row_ref = row.data();
        for (auto el : row_ref) {
            std::cout << el << "\n";
        }
        std::cout << "--------------\n";
    }
}

// reads a big file to clear cache
void clear_cache() {
    hor_table big_file("C:/Users/kbasi/git/db-indexes/examples/big-hor-db");    // this table is used just for clearing cache
    std::vector<row> rows;
    int cnt = big_file.count();
    big_file.read_rows(rows, cnt);
}

int main(int argc, char **argv) {
    // TODO make this command line argument
    clear_cache();
    
    stopwatch sw;
    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    ver_table vtable("C:/Users/kbasi/git/db-indexes/examples/db-ver");

    {
        std::vector<row> rows;
        sw.start();
        htable.read_rows(rows, htable.count());
        std::cout << "Hor table read all rows: " << sw.stop() << "\n";
    }

    {
        std::vector<row> rows;
        sw.start();
        vtable.read_rows(rows, vtable.count());
        std::cout << "Ver table read all rows: " << sw.stop() << "\n";
    }

    return 0;
}