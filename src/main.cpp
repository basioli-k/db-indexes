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

    auto queries = qb.generate_queries({ 0, 1 }, op::lor);
    for (size_t i = 0 ; i < queries.size() ; ++i)
    {
        try{
            std::cout << queries[i].query_text(htable.schema()) << "\n";
            sw.start();
            auto results = htable.execute_query(queries[i]);
            std::cout << sw.stop() << "\n";
            std::cout << "No of rows: " << results.size() << "\n";
        }
        catch (std::exception e) {
            std::cout << e.what() << "\n";
            exit(-1);
        }
        
    }
    

    // na svakoj tablici napraviti funkciju execute koja prima query, bavi se filterima itd itd

    return 0;
}