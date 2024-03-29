#pragma once
#include <fstream>
#include <string>
#include <vector>
#include "row.h"
#include "schema.h"
#include "io_handler.h"
#include "common.h"
#include "table.h"
#include "query.h"
#include "stopwatch.h"

class hor_table : public table {
    io_handler _table_handler;
    int _no_of_reads = 0;
public:
    hor_table(const std::string& table_path) : table(table_path),
        _table_handler(table_path + maybe_backslash(table_path) +  _schema.get_name() + HOR_TABLE_SUFF) {}

    int no_of_reads() {
        _no_of_reads += _table_handler.no_of_reads();
        return _no_of_reads;
    }

    void reset_reads() {
        _table_handler.reset_reads();
        _no_of_reads = 0;
    }
    
    // TODO when writing execution with index don't forget to add no_of_reads from index
    query_res execute_query(query& q) {
        query_res qres(q.qtype());

        _table_handler.seekg(0);
        int32_t total_rows = count();
        qres.reserve(q.limit() ? q.limit() : total_rows);
        auto row_size = _schema.row_size();
        int32_t rows_in_block = BLOCK_SIZE / row_size;

        std::vector<int32_t> rows_data;

        for( ; total_rows ; ) {
            _table_handler.read(rows_data, rows_in_block * row_size / 4);
            for(size_t i = 0; i < rows_data.size() && total_rows; ) {
                std::vector<int32_t> row_data;
                size_t to_copy = row_size / 4;
                while (to_copy--) row_data.push_back(rows_data[i++]);
                row r(row_data, _schema);

                if (q.is_satisfied(r)){
                    if (q.qtype() == query_type::star)
                        qres.add(r);
                    else {
                        auto val = r.get_val(q.agg_dim());
                        qres.add(val);
                    }
                        
                }
                total_rows--;

                if (qres.size() >= q.limit() && q.limit()) {
                    qres.shrink_to_fit();
                    return qres;
                }
                    
            }
            rows_data.clear();
        }

        qres.shrink_to_fit();
        return qres;
    }

    void query_by_offsets(std::vector<row>& rows, std::vector<int32_t>& offsets) {
        std::sort(offsets.begin(), offsets.end());
        auto row_size = _schema.row_size();
        int32_t rows_in_block = BLOCK_SIZE / row_size;

        std::set<int32_t> blocks_to_read;
        for (auto offset : offsets)
            blocks_to_read.insert((offset / rows_in_block) * BLOCK_SIZE);

        std::vector<int32_t> rows_data;
        size_t offset_ind = 0;
        for (auto block : blocks_to_read) {
            _table_handler.seekg(block);
            _table_handler.read(rows_data, rows_in_block * row_size / 4);
            while ( offset_ind < offsets.size() && (offsets[offset_ind] / rows_in_block) * BLOCK_SIZE == block ) {
                size_t rw_dat_ind = (offsets[offset_ind] * (row_size / 4)) % rows_data.size();
                std::vector<int32_t> row_data;
                size_t to_copy = row_size / 4;
                while (to_copy--) row_data.push_back(rows_data[rw_dat_ind++]);
                row r(row_data, _schema);
                rows.emplace_back(r);
                offset_ind++;
            }
            rows_data.clear();
        }
    }


    // read rows_num rows, starting at offset row_offset (offset meaning the cardinal number of the row in table)
    void read_rows(std::vector<row>& rows, int32_t rows_num, size_t row_offset = 0) {
        int32_t avail = count() - row_offset;
        
        if (avail <= 0) return;

        rows_num = std::min(int32_t(rows_num), avail);
        rows.reserve(rows.size() + rows_num);
        auto row_size = _schema.row_size();

        // size_t file_offset = row_offset * row_size;
        int32_t rows_in_block = BLOCK_SIZE / row_size;
        _table_handler.seekg((row_offset / rows_in_block) * BLOCK_SIZE);  // locate the block where the offset is located
        // std::cout << (file_offset / BLOCK_SIZE) * BLOCK_SIZE << "\n";
        std::vector<int32_t> rows_data;
        
        // row_size / 4 because row_data is a vector of 32 bit ints
        bool first_read = true;
        for(;rows_num;) {
            _table_handler.read(rows_data, rows_in_block * row_size / 4);

            std::vector<int32_t> row_data;
            size_t i = first_read ? (row_offset % rows_in_block) * row_size / 4 : 0;  // on first read we may need to skip a few rows
            first_read = false;
            for(; i < rows_data.size() && rows_num; ) {
                size_t to_copy = row_size / 4;
                while (to_copy--) row_data.push_back(rows_data[i++]);
                // TODO potencijalno filtriranje ce doci ovdje
                rows.emplace_back(row_data, _schema); // row_data will be moved, no need to clear it
                row_data.reserve(row_size / 4);
                rows_num--;
            }
            rows_data.clear();
        }
    }

    void insert(row& new_row) {
    //     _table_handler.seekg_to_end();
    //     _table_handler.write(new_row.data());
    //     increment_count();  // TODO only if successful
    }
};