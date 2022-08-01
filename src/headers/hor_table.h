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
public:
    int reads_num = 0;
    hor_table(const std::string& table_path) : table(table_path),
        _table_handler(table_path + maybe_backslash(table_path) +  _schema.get_name() + HOR_TABLE_SUFF) {}

    // TODO make this function have different return values
    std::vector<row> execute_query(query& q) {
        std::vector<row> rows;

        _table_handler.seekg(0);
        int32_t total_rows = count();
        rows.reserve(q.limit() ? q.limit() : total_rows);
        auto row_size = _schema.row_size();
        int32_t rows_in_block = BLOCK_SIZE / row_size;

        std::vector<int32_t> rows_data;

        for( ; total_rows ; ) {
            reads_num++;
            _table_handler.read(rows_data, rows_in_block * row_size / 4);
            for(size_t i = 0; i < rows_data.size() && total_rows; ) {
                std::vector<int32_t> row_data;
                size_t to_copy = row_size / 4;
                while (to_copy--) row_data.push_back(rows_data[i++]);
                row r(row_data, _schema);

                if (q.is_satisfied(r)){
                    rows.emplace_back(std::move(r));
                }
                total_rows--;

                if (rows.size() >= q.limit() && q.limit()) {
                    rows.shrink_to_fit();
                    return rows;
                }
                    
            }
            rows_data.clear();
        }

        rows.shrink_to_fit();
        return rows;
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