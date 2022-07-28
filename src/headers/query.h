#pragma once
#include <set>
#include "schema.h"
#include "filter.h"

constexpr size_t INTERVALS_NUM = 3;

class query {
    std::set<int> _query_dims;
    filter_ptr _filter;
    uint32_t _limit;
    
public:
    query(filter_ptr f, uint32_t row_limit = 0) : _filter(std::move(f)), _limit(row_limit) {
        _filter->get_filter_dims(_query_dims);
    }

    uint32_t limit() { return _limit; }
    bool is_satisfied(row& row) {
        if (!_filter) return true;
        return _filter->apply(row);
    }

    bool is_satisfied(std::unordered_map<int, const db_val>& dim_vals) {
        if (!_filter) return true;
        return _filter->apply(dim_vals);
    }

    std::string query_text(schema& schema) {
        return _filter->get_filter_text(schema);
    }
};

using interval = std::pair<db_val, db_val>;
class dist {
    db_val _left;
    db_val _right;
public:
    dist (const db_val& left, const db_val& right) : _left(left), _right(right) {}

    std::vector<interval> get_intervals() {
        std::vector<interval> intervals;
        intervals.reserve(INTERVALS_NUM);
        auto step = (_right - _left) / INTERVALS_NUM;
        auto prev = _left;
        auto cur = _left + step;

        for (int i = 0; i < INTERVALS_NUM; ++i) {
            intervals.emplace_back(prev, std::min(cur, _right));
            prev = cur;
            cur = prev + step;
        }

        return intervals;
    }
};

// loads distribution and generates queries
class query_builder {
    using intervals_vec = std::vector<std::vector<interval>>;

    schema& _schema;
    std::vector<dist> _dists;
public:
    query_builder(const std::string& dist_path, schema& schema) : _schema(schema) {
        std::ifstream dist_file(dist_path);
        assert(dist_file.is_open());
        
        std::string dist_line;
        getline(dist_file, dist_line);
        
        std::vector<std::string> dist_strs;
        boost::algorithm::split(dist_strs, dist_line, boost::algorithm::is_any_of(";"));
        assert(dist_strs.size() == _schema.col_num());

        for(size_t i = 0 ; i < dist_strs.size(); ++i) {
            std::vector<std::string> dist_data;
            boost::algorithm::split(dist_data, dist_strs[i], boost::algorithm::is_any_of(":"));
            if (dist_data[0] == "G") {
                auto mi = str_to_db_val(dist_data[1], _schema.get_column(i).type);
                auto sig = str_to_db_val(dist_data[2], _schema.get_column(i).type);
                _dists.emplace_back(mi - sig, mi + sig);
            }
            else if (dist_data[0] == "U") {
                auto left = str_to_db_val(dist_data[1], _schema.get_column(i).type);
                auto right = str_to_db_val(dist_data[2], _schema.get_column(i).type);
                _dists.emplace_back(left, right);
            }
            else throw std::exception("Invalid distribution\n");
        }
    }

    // given a vector of dimensions create queries with filters over those dimensions
    // the filters will have the format:
    // (col_1 > lower_bound_1 and col_1 < upper_bound_1) and/or (col_2 > lower_bound_2 and col_2 < upper_bound_2) and/or ...
    // second parameter specifies wheter to use and or or
    std::vector<query> generate_queries(const std::vector<int>& col_dims, op logical_op) {
        if (logical_op != op::land && logical_op != op::lor) 
            throw std::exception("Only use logical operators in generate queries\n");

        std::vector<query> queries;
        intervals_vec col_intervals;
        col_intervals.resize(col_dims.size());
        
        for (size_t i = 0 ; i < col_intervals.size(); ++i) {
            col_intervals[i].reserve(INTERVALS_NUM);
            col_intervals[i] = _dists[col_dims[i]].get_intervals();
        }
        
        intervals_vec all_combs;

        get_all_combinations(all_combs, col_intervals, {}, 0);

        for(auto& filt_ints : all_combs) {
            // auto query_filter = make_query_filter(col_dims, filt_ints, logical_op);
            queries.push_back(make_query_filter(col_dims, filt_ints, logical_op));
        }

        return queries;
    }
private:
    void get_all_combinations(intervals_vec& all_combs, intervals_vec& col_intervals,
        std::vector<interval> curr, size_t next_col)
    {
        if (next_col >= col_intervals.size()) {
            all_combs.emplace_back(std::move(curr));
            return;
        }

        for (size_t i = 0 ; i < col_intervals[next_col].size(); ++i) {
            std::vector<interval> copy = curr;
            copy.push_back(col_intervals[next_col][i]);
            get_all_combinations(all_combs, col_intervals, copy, next_col + 1);
        }
    }

    std::unique_ptr<filter> make_query_filter(
        const std::vector<int>& col_dims, 
        std::vector<interval>& intervals, op log_op
    ) {
        
        std::vector<filter_ptr> filters;
        filters.reserve(intervals.size());

        for(size_t i = 0 ; i < intervals.size(); ++i) {
            std::vector<filter_ptr> tmp;
            tmp.reserve(2);
            auto left = std::make_unique<filter>(op::gt, std::move(std::vector<filter_ptr>()), intervals[i].first, col_dims[i]);
            auto right = std::make_unique<filter>(op::lt, std::move(std::vector<filter_ptr>()), intervals[i].second, col_dims[i]);
            tmp.emplace_back(std::move(left));
            tmp.emplace_back(std::move(right));
            filters.emplace_back( std::make_unique<filter>(op::land, std::move(tmp)) );
        }

        return std::make_unique<filter>(log_op, std::move(filters));
    }
};

// query ideas

// get entire record filtered by one column

// get entire record filtered by two columns

// get sum of entire column

// get sum of entire column filtered by that column

// get sum of entire column filtered by other column

// get sum of entire column filtered by two columns