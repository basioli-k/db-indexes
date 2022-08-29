#pragma once

#include "schema.h"
#include "io_handler.h"
#include "common.h"
#include "row.h"
#include <vector>
#include <limits>


class ver_table : public table {
    std::vector<io_handler> _col_handlers;
    std::vector<size_t> _col_sizes;
    std::vector<int32_t> _entries_in_block;

public:
    ver_table(const std::string& table_path) : table(table_path)
    {
        auto col_num = _schema.col_num();
        _col_handlers.reserve(col_num);
        for(int dim = 0; dim < col_num; ++dim)
            _col_handlers.emplace_back(table_path + maybe_backslash(table_path) + _schema.get_name() + "/" + _schema.get_column(dim).name + VER_TABLE_SUFF);

        // precompute these values
        for (size_t dim = 0 ; dim < _col_handlers.size() ; ++dim) {
            _col_sizes.push_back(get_size(_schema.get_column(dim).type) / 4);
            _entries_in_block.push_back(BLOCK_SIZE / (_col_sizes[dim] * 4));
        }
    }

    int no_of_reads() {
        return std::accumulate(_col_handlers.begin(), _col_handlers.end(), 0, [](int sum, auto& col){ return sum + col.no_of_reads(); });
    }

    int reset_reads() {
        for(auto& col : _col_handlers)
            col.reset_reads();
    }

    query_res execute_query(query& q) {
        query_res qres(q.qtype());

        int32_t total_rows = count();
        qres.reserve(q.limit() ? q.limit() : total_rows);
        auto q_dims = q.get_q_dims();

        std::vector<int32_t> col_index = _entries_in_block;// track index of processed entries in col
        // queried cols are considered processed 

        std::vector<std::vector<int32_t>> cols; // don't forget to clear this before reading new bytes
        cols.resize(_col_handlers.size());
        // reserve space
        for (size_t dim = 0 ; dim < cols.size() ; ++dim)
            cols[dim].reserve(_entries_in_block[dim] * _col_sizes[dim]);
        
        // set query file handlers to start of file
        for(auto dim : q_dims)
            _col_handlers[dim].seekg(0);
        
        for(size_t row_index = 0; row_index < total_rows ;) {
            std::vector<size_t> rows_to_use;

            if (q_dims.size()) {
                // get columns referenced in query
                for (auto dim : q_dims) {
                    if (col_index[dim] < _entries_in_block[dim])
                        continue;
                    col_index[dim] = 0;
                    cols[dim].clear();
                    _col_handlers[dim].read(cols[dim], _entries_in_block[dim] * _col_sizes[dim]);
                }
                // get smallest no of _entries_in_block from queried dims, that is the no of rows we process
                // in a block   
                int32_t min_entries_in_block = std::numeric_limits<int32_t>::max();
                for (auto dim : q_dims)
                    min_entries_in_block = std::min(_entries_in_block[dim], min_entries_in_block);

                // evaluate query
                for (size_t i = 0 ; i < min_entries_in_block && row_index < total_rows; ++i) {
                    std::vector<db_val> values(_col_handlers.size());

                    for (auto dim : q_dims)
                        values[dim] = get_value(cols[dim].data() + (col_index[dim] + i * _col_sizes[dim]), _schema.get_column(dim).type);

                    if (q.is_satisfied(values)) {
                        //TODO ovdje ce se odradivati agregacijske funkcije
                        if (q.qtype() == query_type::star)
                            rows_to_use.push_back(row_index);
                        else if (q.qtype() == query_type::sum)
                            qres.add(values[q.agg_dim()]);
                    }
                    ++row_index;
                }

                // update the state of the queried col indexes in cols vector
                for (auto dim : q_dims)
                    col_index[dim] += min_entries_in_block;
            }
            else {
                rows_to_use.push_back(0);
                rows_to_use.push_back(q.limit() ? q.limit() : total_rows);
            }

            if (!rows_to_use.size()) // this basically means we are aggregating
                continue;

            // decide which blocks to read
            // read remaining cols
            for (size_t dim = 0; dim < cols.size(); ++dim) {
                if (q_dims.contains(dim)) continue; // skip queried dims
                _col_handlers[dim].seekg((rows_to_use[0] / _entries_in_block[dim]) * BLOCK_SIZE);
                
                int blocks_to_read = (rows_to_use[rows_to_use.size() - 1] / _entries_in_block[dim]) -
                    (rows_to_use[0] / _entries_in_block[dim]) + 1;

                cols[dim].clear();
                while (blocks_to_read--) {    // force multiple block reads
                    _col_handlers[dim].read(cols[dim], _entries_in_block[dim] * _col_sizes[dim]);
                }
            }
            
            if (q_dims.empty()) {
                for (size_t row_off = rows_to_use[0] ; row_off < rows_to_use[1]; ++row_off) {
                    prepare_row_data(qres, cols, row_off);
                    
                    if (qres.size() >= q.limit() && q.limit()) {
                        qres.shrink_to_fit();
                        return qres;
                    }
                }
            } 
            else {
                // prepare row data
                for (auto row_off : rows_to_use) {
                    prepare_row_data(qres, cols, row_off);
                    
                    if (qres.size() >= q.limit() && q.limit()) {
                        qres.shrink_to_fit();
                        return qres;
                    }
                }
            }

            
        }
        qres.shrink_to_fit();
        return qres;
    }

    // returns list of entries
    void read_cols(std::vector<int32_t>& buff, int dim, int32_t cols_num, size_t col_offset = 0) {
        int32_t avail = count() - col_offset;
        
        if (avail <= 0) return;
        cols_num = std::min(int32_t(cols_num), avail);

        auto col_size = get_size(_schema.get_column(dim).type);
        
        int32_t entries_in_block = BLOCK_SIZE / col_size;
        _col_handlers[dim].seekg((col_offset / entries_in_block) * BLOCK_SIZE);
        
        bool first_read = true;
        std::vector<int32_t> first_read_buff;
        for (;cols_num > 0;) {
            if (first_read) {
                _col_handlers[dim].read(first_read_buff, std::min(entries_in_block, cols_num )* col_size / 4);
                for (size_t i = (col_offset % entries_in_block) * col_size / 4; i < first_read_buff.size() && cols_num > 0;) {
                    auto to_copy = col_size / 4;
                    while(to_copy--) buff.push_back(first_read_buff[i++]);
                    cols_num--;
                }
                first_read = false;
            }
            else {
                _col_handlers[dim].read(buff, std::min(entries_in_block, cols_num ) * col_size / 4);
                cols_num -= entries_in_block;
            }
                
        }
    }

    void read_rows(std::vector<row>& rows, int32_t rows_num, size_t row_offset = 0) {
        int32_t avail = count() - row_offset;
        
        if (avail <= 0) return;

        rows_num = std::min(int32_t(rows_num), avail);
        rows.reserve(rows.size() + rows_num);

        std::vector<std::vector<int32_t>> cols;
        cols.resize(_col_handlers.size());
        
        // reserve space
        for (size_t dim = 0 ; dim < cols.size() ; ++dim)
            cols[dim].reserve(rows_num * get_size(_schema.get_column(dim).type) / 4);
        
        // read columns
        for(size_t dim = 0; dim < cols.size(); ++dim)
            read_cols(cols[dim], dim, rows_num, row_offset);

        // copy to rows
        std::vector<size_t> cols_indexes(cols.size());
        std::vector<int32_t> row_data;
        
        for (size_t i = 0; i < rows_num; ++i) {
            row_data.reserve(_schema.row_size() / 4);
            for (size_t dim = 0 ; dim < cols.size(); ++dim) {
                auto to_copy = get_size(_schema.get_column(dim).type) / 4;
                while(to_copy--) row_data.emplace_back(cols[dim][cols_indexes[dim]++]);
            }
            rows.emplace_back(row_data, _schema);
            row_data.clear();
        }
    }

    void insert(row& new_row) {
        // auto beg = new_row.data().begin();

        // for(size_t dim = 0; dim < _schema.col_num(); ++dim) {
        //     auto size = get_size(_schema.get_column(dim).type) / 4;
        //     _col_handlers[dim].seekg_to_end();
        //     // _col_handlers[dim].write(beg, size);
        //     beg = beg + size;
        // }
        // increment_count();  // TODO only if successful
    }

private:
    void prepare_row_data(query_res& qres, std::vector<std::vector<int32_t>>& cols, size_t row_off) {
        std::vector<int32_t> row_data;
        for (size_t dim = 0 ; dim < cols.size(); ++dim) {
            auto to_copy = _col_sizes[dim];
            auto col_dim_offset = (row_off * _col_sizes[dim]) % cols[dim].size();
            while(to_copy--) row_data.emplace_back(cols[dim][col_dim_offset++]);
        }
        row r(row_data, _schema);
        qres.add(r);
    }
};