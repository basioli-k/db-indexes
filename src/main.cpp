#include <iostream>
#include <vector>
#include <thread>
#include <fstream>
#include "headers/hor_table.h"
#include "headers/ver_table.h"
#include "headers/row.h"
#include "headers/stopwatch.h"
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

bool equal(row& r1, row& r2) {
    for (int i = 0 ; i < r1.data().size(); ++i) {
        if (r1.data()[i] != r2.data()[i]) return false;
    }
    return true;
}

int main(int argc, char **argv) {
    stopwatch sw;
    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    ver_table vtable("C:/Users/kbasi/git/db-indexes/examples/db-ver");
    // // filter f(3, op::eq, nullptr, nullptr);

    // // std::unique_ptr<filter_base> f( new filter<int>(3, op::eq, nullptr, nullptr) );
    {
        for (int i = 0 ; i < htable.count(); ++i) {
            std::vector<row> hrows, vrows;
            htable.read_rows(hrows, 1, i);
            vtable.read_rows(vrows, 1, i);
            if(!equal(hrows[0], vrows[0])) {
                std::cout << "not equal at index[" << i << "]\n";
                break;
            }
        }
        // for (int i = 0 ; i < 4; ++i) {
        //     std::cout << rows[0].get_val(i) << "\n";
        // }
        // std::cout << f->apply(rows[0], 0) << "\n";
    }

    return 0;
}