#pragma once
#include <fstream>
#include <string>
#include <vector>
#include "row.h"
#include "schema.h"
#include "io_handler.h"
#include "common.h"

class hor_table {
    schema _schema;
    io_handler _table_handler;
    io_handler _count_handler;

public: 
    hor_table(const std::string& table_path) {
        auto maybe_backslash = table_path[table_path.size() - 1] == '/' ? "" : "/";
        _schema = schema(table_path + maybe_backslash + DEFAULT_SCHEMA_NAME);
        _table_handler = io_handler(table_path + maybe_backslash +  _schema.get_name() + HOR_TABLE_SUFF);
        _count_handler = io_handler(table_path + maybe_backslash +  _schema.get_name() + CNT_SUFF);
    }

    // read rows_num rows, starting at offset row_offset (offset meaning the cardinal number of the row in table)
    void read_rows(std::vector<row>& rows, uint32_t rows_num, size_t row_offset = 0) {
        int32_t avail = count() - row_offset;
        
        if (avail <= 0) return;

        rows_num = std::min(int32_t(rows_num), avail);
        rows.reserve(rows.size() + rows_num);
        auto row_size = _schema.row_size();

        // TODO usporediti je li mozda bolje procitati sve odjednom i onda raspoređivati u retke
        // vjv je tu problem
        
        std::vector<int32_t> data;
        _table_handler.read(data, rows_num * row_size / 4, row_offset);

        size_t data_index = 0;
        for (size_t i = 0 ; i < rows_num; ++i) {
            std::vector<int32_t> row_data;
            size_t el_num = row_size / 4;
            row_data.reserve(el_num);
            while(el_num--) row_data.emplace_back(data[data_index++]);
            rows.emplace_back(row_data, _schema);
        }

        // for (size_t i = 0; i < rows_num; ++i) {
        //     std::vector<int32_t> row_data;
        //     // row_size / 4 because row_data is a vector of 32 bit ints
        //     _table_handler.read(row_data, row_size / 4, row_size * (row_offset++));
        //     rows.emplace_back(row_data, _schema);
        // }
    }

    void insert(row& new_row) {
        _table_handler.write(new_row.data());
        increment_count();  // TODO only if successful
    }

    int32_t count() {
        return _count_handler.read_one();
    }
private:
    void increment_count() {
        int32_t cnt = count() + 1;
        _count_handler.write_one(cnt, 0);
    }

    void decrement_count() {
        int32_t cnt = count() - 1;
        _count_handler.write_one(cnt, 0);
    }
};