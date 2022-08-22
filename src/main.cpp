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
#include "headers/b_tree.h"

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

void test_queries() {
    stopwatch sw;
    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    ver_table vtable("C:/Users/kbasi/git/db-indexes/examples/db-ver");

    // hor_table htable("E:/db-hor");
    // ver_table vtable("E:/db-ver");

    query_builder qb("C:/Users/kbasi/git/db-indexes/examples/dists/dist1.txt", htable.schema());

     //// with filters
    // auto queries = qb.generate_queries({ 0, 3 }, op::lor, query_type::star);

    // with filters with sum
    auto queries = qb.generate_queries({ 0, 3 }, op::lor, query_type::sum, htable.count(), 0);

    // // without filters
    // std::vector<query> queries;
    // for (int i = 100000 ; i <= htable.count(); i += 100000)
    //     queries.emplace_back(nullptr, query_type::star, i);

    // // without filters with sum
    // std::vector<query> queries;
    // for (int i = 100000 ; i <= htable.count(); i += 100000)
    //     queries.emplace_back(nullptr, query_type::sum, i, 0);

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
                results_eq = results_eq && equal(results.rows()[i], res2.rows()[i]);
            }

            if (results.val() == res2.val())
                std::cout << results.val() << " good sum.\n";
            else
                std::cout << "wrong sum\n";

            std::cout << "rows num: " << results.size() << "\n";
            std::cout << "Results eq variable is: " << results_eq << "\n";
        }
        catch (std::exception e) {
            std::cout << e.what() << "\n";
            exit(-1);
        }
        
    }
}
std::string b_tree_node::path = "C:/Users/kbasi/git/db-indexes/examples/db-hor/index";
int main(int argc, char **argv) {
    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    b_tree btree = create_b_tree(htable, 2);

    // for (int i = 1 ; i <= 10; ++i)
    //     btree.insert(i*5, i);
        
    // btree.traverse();
    return 0;
}