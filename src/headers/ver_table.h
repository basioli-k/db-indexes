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
            _entries_in_block.push_back(BLOCK_SIZE / _col_sizes[dim]);
        }
    }

    std::vector<row> execute_query(query& q) {
        std::vector<row> rows;

        int32_t total_rows = count();
        rows.reserve(total_rows);
        auto q_dims = q.get_q_dims();

        std::vector<int32_t> col_index = _entries_in_block;// track index of processed entries in col
        // queried cols are considered processed 

        std::vector<std::vector<int32_t>> cols; // don't forget to clear this before reading new bytes
        cols.resize(_col_handlers.size());
        // reserve space
        for (size_t dim = 0 ; dim < cols.size() ; ++dim)
            cols[dim].reserve(_entries_in_block[dim] * _col_sizes[dim]);
        
        // get smallest no of _entries_in_block from queried dims, that is the no of rows we process
        // in a block
        int32_t min_entries_in_block = std::numeric_limits<int32_t>::max(); // get biggest size_t by initializing as zero and overflowing by subtraction
        for (auto dim : q_dims)
            min_entries_in_block = std::min(_entries_in_block[dim], min_entries_in_block);

        for(size_t row_index = 0; row_index < total_rows ;) {
            // get columns referenced in query
            for (auto dim : q_dims) {
                if (col_index[dim] < _entries_in_block[dim])
                    continue;
                col_index[dim] = 0;
                cols[dim].clear();
                _col_handlers[dim].read(cols[dim], _entries_in_block[dim] * _col_sizes[dim]);
            }

            std::vector<size_t> rows_to_use;
            // evaluate query
            for (size_t i = 0 ; i < min_entries_in_block; ++i) {
                std::unordered_map<int, const db_val> values;
                for (auto dim : q_dims)
                    values.emplace(dim, get_value(cols[dim].data() + (col_index[dim] + i * _col_sizes[dim]), _schema.get_column(dim).type));

                if (q.is_satisfied(values)) {
                    // ovdje ce se odradivati agregacijske funkcije ili 
                    // pamcenje row indeksa koji zadovoljavaju kveri u slucaju da zelimo sve retke
                    rows_to_use.push_back(row_index);
                    ++row_index;
                }
                ++row_index;
            }

            if (!rows_to_use.size()) return rows;
            // update the state of the queried col indexes in cols vector
            for (auto dim : q_dims)
                col_index[dim] += min_entries_in_block;

            // decide which blocks to read
            // read remaining cols
            for (size_t dim = 0; dim < cols.size(); ++dim) {
                if (q_dims.contains(dim)) continue; // skip queried dims
                _col_handlers[dim].seekg((rows_to_use[0] / _entries_in_block[dim]) * BLOCK_SIZE);
                
                int blocks_to_read = (rows_to_use[rows_to_use.size() - 1] / _entries_in_block[dim]) -
                    (rows_to_use[0] / _entries_in_block[dim]) + 1;

                _col_handlers[dim].read(cols[dim], blocks_to_read * _entries_in_block[dim] * _col_sizes[dim]);
            }
            

            // prepare row data
            for (auto row_off : rows_to_use) {
                std::vector<int32_t> row_data;
                // indeks vrijednosti retka krecu na:
                for (size_t dim = 0 ; dim < cols.size(); ++dim) {
                    auto to_copy = _col_sizes[dim];
                    auto col_dim_offset = (row_off * _col_sizes[dim]) % cols[dim].size();   // treba viditi je li ovo dobro izracunato
                    while(to_copy--) row_data.emplace_back(cols[dim][col_dim_offset++]);
                }
                rows.emplace_back(row_data, _schema);

                if (rows.size() >= q.limit() && q.limit()) {
                    rows.shrink_to_fit();
                    return rows;
                }
            }
        }
        rows.shrink_to_fit();
        return rows;
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
                _col_handlers[dim].read(first_read_buff, entries_in_block * col_size / 4);
                for (size_t i = (col_offset % entries_in_block) * col_size / 4; i < first_read_buff.size() && cols_num > 0;) {
                    auto to_copy = col_size / 4;
                    while(to_copy--) buff.push_back(first_read_buff[i++]);
                    cols_num--;
                }
                first_read = false;
            }
            else 
                _col_handlers[dim].read(buff, entries_in_block * col_size / 4);
            cols_num -= entries_in_block;
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
};