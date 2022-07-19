#include <iostream>
#include <vector>
#include <thread>
#include "headers/hor_table.h"
#include "headers/ver_table.h"
#include "headers/row.h"
#include "headers/stopwatch.h"
#include "headers/io_handler.h"
// #include "headers/filter.h"

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
    hor_table big_file("C:/Users/kbasi/git/db-indexes/examples/cache-clear-db");    // this table is used just for clearing cache
    std::vector<row> rows;
    int cnt = big_file.count() / 4;
    big_file.read_rows(rows, cnt);
}

void test_read_speed() {
    stopwatch sw;
    std::cout << "hej\n";
    sw.start();
    clear_cache();
    std::cout << "cache clear takes: " << sw.stop() << "\n";

    for (int j = 0 ; j < 100 ; ++j) {
        std::fstream output("C:/Users/kbasi/git/results" + std::to_string(j) + ".txt", std::ios::app | std::ios::out);
        for (uint64_t i = 1 ; i < 2450000000 ; i *= 2) {
            clear_cache();
            std::fstream file("C:/Users/kbasi/git/glupost.txt", std::ios::binary | std::ios::in | std::ios::out);
            char* buff = new char[i];
            sw.start();
            file.read(buff, i);
            output << i << "," << sw.stop() << "\n";
            delete[] buff;
        }
        std::cout << "Finished " << j + 1 << "/100.\n";
    }
}

int main(int argc, char **argv) {
    test_read_speed();
    // clear_cache();
    // stopwatch sw;
    // hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    // ver_table vtable("C:/Users/kbasi/git/db-indexes/examples/db-ver");
    // filter f(3, op::eq, nullptr, nullptr);

    // std::unique_ptr<filter_base> f( new filter<int>(3, op::eq, nullptr, nullptr) );
    // {
    //     std::vector<row> rows;
    //     sw.start();
    //     htable.read_rows(rows, 3);
    //     std::cout << "Hor table read all rows: " << sw.stop() << "\n";
    //     for (int i = 0 ; i < 4; ++i) {
    //         std::cout << rows[0].get_val(i) << "\n";
    //     }
    //     // std::cout << f->apply(rows[0], 0) << "\n";
    // }

    // {
    //     std::vector<row> rows;
    //     sw.start();
    //     vtable.read_rows(rows, 3);
    //     std::cout << "Ver table read all rows: " << sw.stop() << "\n";
    //     print_rows(rows);
    // }

    return 0;
}