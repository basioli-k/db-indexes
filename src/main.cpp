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
    for (size_t i = 0 ; i < rows.size() ; ++i) {
        // for (auto el : rows[i].data()) {
        //     std::cout << el << "\n";
        // }
        rows[i].print_values();    
        std::cout << "--------------\n";
        break;
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

    // hor_table htable("E:/db-hor");
    // ver_table vtable("E:/db-ver");

    query_builder qb("C:/Users/kbasi/git/db-indexes/examples/dists/dist1.txt", htable.schema());

    // auto queries = qb.generate_queries({ 0, 3 }, op::lor, query_type::star);

    std::vector<query> queries;
    for (int i = htable.count() ; i <= htable.count(); i += 100000)
        queries.emplace_back(nullptr, query_type::star, i);

    // new way of reading (using queries)
    for (size_t i = 0 ; i < queries.size() ; ++i)
    {
        try{
            std::cout << queries[i].query_text(htable.schema()) << "\n";
            sw.start();
            auto results = htable.execute_query(queries[i]);
            std::cout << "query hor: " << sw.stop() << "\n";
            std::cout << "reads: " << htable.reads_num << "\n";
            htable.reads_num = 0;
            sw.start();
            auto res2 = vtable.execute_query(queries[i]);
            std::cout << "query ver: " << sw.stop() << "\n";
            std::cout << "reads: " << vtable.reads_num << "\n";
            vtable.reads_num = 0;
            bool results_eq = results.size() == res2.size();
            for (int i = 0 ; i < results.size(); ++i) {
                results_eq = results_eq && equal(results[i], res2[i]);
                if (!results_eq) {
                    results[i].print_values();
                    std::cout << "..........\n";
                    res2[i].print_values();
                    std::cout << "..........\n";
                    std::cout << i << "\n";
                }

            }
            std::cout << "rows num: " << results.size() << "\n";
            std::cout << "Results eq variable is: " << results_eq << "\n";
        }
        catch (std::exception e) {
            std::cout << e.what() << "\n";
            exit(-1);
        }
        
    }
    
    // std::vector<row> hrows, vrows;
    // sw.start();
    // htable.read_rows(hrows, htable.count());
    // std::cout << "hor: " << sw.stop() << "\n";
    // sw.start();
    // vtable.read_rows(vrows, vtable.count());
    // std::cout << "ver: " << sw.stop() << "\n";

    // bool results_eq = hrows.size() == vrows.size();
    // for (int i = 0 ; i < hrows.size(); ++i) {
    //     results_eq = results_eq && equal(hrows[i], vrows[i]);
    //     if (!results_eq) {
    //         // results[i].print_values();
    //         // std::cout << "..........\n";
    //         // res2[i].print_values();
    //         // std::cout << "..........\n";
    //         std::cout << i << "\n";
    //     }
    // }

    return 0;
}