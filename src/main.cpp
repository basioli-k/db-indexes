#include <iostream>
#include <vector>
#include <thread>
#include <memory>
#include <fstream>
#include "headers/hor_table.h"
#include "headers/ver_table.h"
#include "headers/row.h"
#include "headers/stopwatch.h"
#include "headers/filter.h"
#include "headers/query.h"
#include "headers/b_tree.h"
#include "headers/hash_index.h"

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
            sw.start();
            auto res2 = vtable.execute_query(queries[i]);
            std::cout << "query ver: " << sw.stop() << "\n";

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

void test_b_tree() {
    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    // b_tree btree = create_b_tree(htable, 2);

    b_tree btree(NODE_PARAM);

    auto res = btree.search_range(0, 10000);

    // FOR TESTING IF OFFSETS ARE CORRECT
    // query q(nullptr, query_type::star, 0); // select all
    // auto res = htable.execute_query(q);

    // for(size_t i = 0 ; i < res.size(); ++i) {
    //     auto val = res.rows()[i].get_val(2);
    //     if (btree.search(int32_t(val)) != i) {
    //         std::cout << "greska " << int32_t(val) << " " << i << "\n";
    //         break;
    //     }
    // }
}

void test_hash() {
    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");

    // hash_index hind = create_hash_index(htable, 2);
    hash_index hind((htable.count() / ENTRIES_PER_BLOCK) + 1);
    
    query q(nullptr, query_type::star, 0); // select all
    auto res = htable.execute_query(q);

    for(size_t i = 0 ; i < res.size(); ++i) {
        auto val = res.rows()[i].get_val(2);
        if (hind.search(int32_t(val)) != i) {
            std::cout << "greska " << int32_t(val) << " " << i << "\n";
            break;
        }
    }
    std::cout << "Done\n";
}

void test_b_tree_query() {
    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    b_tree btree(NODE_PARAM);

    auto offsets = btree.search_range(0, 10000);
    std::vector<filter_ptr> vec;

    auto low = std::make_unique<filter>(op::gt, std::move(vec), db_val((int32_t) -1), 2);
    auto high = std::make_unique<filter>(op::lt, std::move(vec), db_val((int32_t) 10001), 2);

    vec.push_back(std::move(low));
    vec.push_back(std::move(high));

    auto fil = std::make_unique<filter>(op::land, std::move(vec));
    query q(std::move(fil), query_type::star, 0);

    auto no_index_rows = htable.execute_query(q);

    std::vector<row> btree_rows;

    htable.query_by_offsets(btree_rows, offsets);

    bool results_eq = btree_rows.size() == no_index_rows.size();
    for (int i = 0 ; i < no_index_rows.size(); ++i) {
        results_eq = results_eq && equal(no_index_rows.rows()[i], btree_rows[i]);
    }

    std::cout << "rows num: " << no_index_rows.size() << "\n";
    std::cout << "Results eq variable is: " << results_eq << "\n";
}

std::string b_tree_node::path = "C:/Users/kbasi/git/db-indexes/examples/db-hor/btree";
std::string bucket_block::path = "C:/Users/kbasi/git/db-indexes/examples/db-hor/hash_ind";

int main(int argc, char **argv) {
    // put indexes in execute query
    // test
    // execute real tests and measure performance
    
    test_hash();

    return 0;
}