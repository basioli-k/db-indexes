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

constexpr const char* RESULTS_FOLDER = "C:/Users/kbasi/git/db-indexes/results/";

void no_index_test(hor_table& htable, ver_table& vtable, query& q, const std::string& test_name) {
    std::ofstream file(RESULTS_FOLDER + test_name + ".txt",  std::ios_base::out |  std::ios_base::app);
    stopwatch sw;

    {
        sw.start();
        auto hres = htable.execute_query(q);
        auto htime = sw.stop();
        auto hreads = htable.no_of_reads();
        file << htime << "," << hreads << ",";
    }
    
    {
        sw.start();
        auto vres = vtable.execute_query(q);
        auto vtime = sw.stop();
        auto vreads = vtable.no_of_reads();
        file << vtime << "," << vreads << "\n";
    }
}

void select_all() {
    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    ver_table vtable("C:/Users/kbasi/git/db-indexes/examples/db-ver");
    query q(nullptr, query_type::star, 0);

    no_index_test(htable, vtable, q, "select_all");
}

void filter_by_one_col() {
    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    ver_table vtable("C:/Users/kbasi/git/db-indexes/examples/db-ver");

    std::vector<filter_ptr> tmp;
    auto low = std::make_unique<filter>(op::gt, std::move(tmp), db_val((double) 0.33), 1);
    auto high = std::make_unique<filter>(op::lt, std::move(tmp), db_val((double) 0.7), 1);
    tmp.push_back(std::move(low));
    tmp.push_back(std::move(high));

    auto fil = std::make_unique<filter>(op::land, std::move(tmp));
    query q(std::move(fil), query_type::star, 0);

    no_index_test(htable, vtable, q, "filter_by_one_col");
}

void filter_by_two_col() {
    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    ver_table vtable("C:/Users/kbasi/git/db-indexes/examples/db-ver");

    std::vector<filter_ptr> tmp;
    auto first = std::make_unique<filter>(op::gt, std::move(tmp), db_val((int32_t) 1700000), 0);
    auto second = std::make_unique<filter>(op::lt, std::move(tmp), db_val((int64_t) -3300543), 3);
    tmp.push_back(std::move(first));
    tmp.push_back(std::move(second));

    auto fil = std::make_unique<filter>(op::lor, std::move(tmp));
    query q(std::move(fil), query_type::star, 0);

    no_index_test(htable, vtable, q, "filter_by_two_col");
}

void select_all_sum() {
    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    ver_table vtable("C:/Users/kbasi/git/db-indexes/examples/db-ver");
    query q(nullptr, query_type::sum, 0, 2);

    no_index_test(htable, vtable, q, "select_all_sum");
}

void filter_by_one_col_sum() {
    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    ver_table vtable("C:/Users/kbasi/git/db-indexes/examples/db-ver");

    std::vector<filter_ptr> tmp;
    auto low = std::make_unique<filter>(op::gt, std::move(tmp), db_val((double) 0.33), 1);
    auto high = std::make_unique<filter>(op::lt, std::move(tmp), db_val((double) 0.7), 1);
    tmp.push_back(std::move(low));
    tmp.push_back(std::move(high));

    auto fil = std::make_unique<filter>(op::land, std::move(tmp));
    query q(std::move(fil), query_type::sum, 0, 1);

    no_index_test(htable, vtable, q, "filter_by_one_col_sum");
}

// call based on multiple placements on disk*
void no_index_prim_key(db_val& val) {
    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    ver_table vtable("C:/Users/kbasi/git/db-indexes/examples/db-ver");

    std::vector<filter_ptr> tmp;
    auto fil = std::make_unique<filter>(op::eq, std::move(tmp), val, 0);
    query q(std::move(fil), query_type::star, 1);

    no_index_test(htable, vtable, q, "no_index_prim_key");
}

// call based on multiple placements on disk
void index_search_test(db_val val) {
    const std::string test_name = "/primary/index_search_test.txt";
    std::ofstream file(RESULTS_FOLDER + test_name, std::ios_base::out |  std::ios_base::app);

    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    b_tree btree(NODE_PARAM);
    hash_index hind((htable.count() / ENTRIES_PER_BLOCK) + 1);

    std::vector<filter_ptr> tmp;
    auto fil = std::make_unique<filter>(op::eq, std::move(tmp), val, 0);
    query q(std::move(fil), query_type::star, 1); // row limit 1 makes us stop when we find result
    stopwatch sw;
    
    // without index
    {
        sw.start();
        auto hres = htable.execute_query(q);
        auto htime = sw.stop();
        auto hreads = htable.no_of_reads();
        htable.reset_reads();
        file << htime << "," << hreads << ",";
    }

    // with btree
    {
        std::vector<row> btree_rows;
        sw.start();
        auto offset = btree.search(int32_t(val));
        assert(offset != -1);
        std::vector<int32_t> btree_off { offset };
        htable.query_by_offsets(btree_rows, btree_off);
        auto btree_time = sw.stop();
        auto btree_reads = htable.no_of_reads() + btree.no_of_reads();
        htable.reset_reads();
        file << btree_time << "," << btree_reads << ",";
    }

    // with hashtable
    {
        std::vector<row> hash_rows;
        sw.start();
        auto offset = hind.search(int32_t(val));
        assert(offset != -1);
        std::vector<int32_t> hash_off { offset };
        htable.query_by_offsets(hash_rows, hash_off);
        auto hash_time = sw.stop();
        auto hash_reads = htable.no_of_reads() + hind.no_of_reads();
        htable.reset_reads();
        file << hash_time << "," << hash_reads << "\n";
    }
}


void index_search_range_test() {
    const std::string test_name = "/primary/index_search_range_test.txt";
    std::ofstream file(RESULTS_FOLDER + test_name, std::ios_base::out |  std::ios_base::app);

    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    b_tree btree(NODE_PARAM);
    constexpr int32_t LOW = 8623;
    constexpr int32_t HIGH = 11546;

    std::vector<filter_ptr> vec;

    auto low = std::make_unique<filter>(op::gt, std::move(vec), db_val((int32_t) LOW), 0);
    auto high = std::make_unique<filter>(op::lt, std::move(vec), db_val((int32_t) HIGH), 0);

    vec.push_back(std::move(low));
    vec.push_back(std::move(high));

    auto fil = std::make_unique<filter>(op::land, std::move(vec));
    query q(std::move(fil), query_type::star, 0);

    stopwatch sw;
    // no index
    {
        sw.start();
        auto hres = htable.execute_query(q);
        auto htime = sw.stop();
        auto hreads = htable.no_of_reads();
        htable.reset_reads();
        file << htime << "," << hreads << ",";
    }

    // btree index
    {
        std::vector<row> btree_rows;
        sw.start();
        auto offsets = btree.search_range(LOW, HIGH);
        htable.query_by_offsets(btree_rows, offsets);
        auto btree_time = sw.stop();
        auto btree_reads = htable.no_of_reads() + btree.no_of_reads();
        htable.reset_reads();
        file << btree_time << "," << btree_reads << "\n";
    }
}

// return various primary keys at different offsets
std::vector<db_val> analyze_table() {
    std::vector<db_val> stat;

    hor_table htable("C:/Users/kbasi/git/db-indexes/examples/db-hor");
    query q(nullptr, query_type::star, 0);
    auto res = htable.execute_query(q);

    for(size_t i = 0 ; i < res.size(); i += htable.count() / 10) {
        auto val = res.rows()[i].get_val(0);
        stat.emplace_back(val);
    }
        
    auto last_val = res.rows()[res.size() - 1].get_val(0);
    stat.emplace_back(last_val);

    return stat;

}

std::string b_tree_node::path = "C:/Users/kbasi/git/db-indexes/examples/db-hor/btree";
std::string bucket_block::path = "C:/Users/kbasi/git/db-indexes/examples/db-hor/hash_ind";

int main(int argc, char **argv) {
    auto stats = analyze_table();

    select_all();
    std::cout << "select all\n";
    filter_by_one_col();
    std::cout << "filter by one col\n";
    filter_by_two_col();
    std::cout << "filter by two col\n";
    select_all_sum();
    std::cout << "select all sum\n";
    filter_by_one_col_sum();
    std::cout << "filter by one col sum\n";

    for (auto& stat : stats) {
        no_index_prim_key(stat);
        index_search_test(stat);
    }
    
    std::cout << "prim key search\n";

    index_search_range_test();
    std::cout << "range\n";
    return 0;
}