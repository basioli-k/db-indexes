#include <iostream>
#include <vector>
#include <thread>
#include <fstream>
#include "headers/hor_table.h"
#include "headers/ver_table.h"
#include "headers/row.h"
#include "headers/stopwatch.h"
#include "headers/filter.h"
#include "headers/query.h"

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

    query_builder qb("C:/Users/kbasi/git/db-indexes/examples/dists/dist1.txt", htable.schema());

    auto queries = qb.generate_queries({ 0, 1 }, op::land);

    // // po prvom retku veci od 300 i manji od 500
    // // po drugom retku manji od 0.3
    // std::unique_ptr<filter> first_col_gt(new filter(op::gt, nullptr, nullptr, db_val(uint64_t(300)), 0 ));
    // std::unique_ptr<filter> first_col_lt(new filter(op::lt, nullptr, nullptr, db_val(uint64_t(500)), 0 ));
    // std::unique_ptr<filter> first_col( new filter(op::land, std::move(first_col_gt), std::move(first_col_lt)));

    // std::unique_ptr<filter> second_col( new filter(op::gt, nullptr, nullptr, db_val(double(0.3)), 1));

    // std::unique_ptr<filter> fil ( new filter(op::lor, std::move(first_col), std::move(second_col)) );
    
    // std::vector<row> rows;
    // int valid = 0;
    // for(int i = 0 ; i < 10 ; ++i) {
    //     htable.read_rows(rows, 1, i);
    //     if (fil->apply(rows[0])) {
    //         valid++;
    //     }   
    //     rows[0].print_values();
    //     std::cout << "-----------------\n";
    //     rows.clear();
    // }

    // std::cout << valid << "\n";

    return 0;
}