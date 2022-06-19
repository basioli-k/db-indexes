#pragma once

#include "schema.h"
#include "io_handler.h"
#include "common.h"
#include "row.h"
#include <vector>


class ver_table {
    schema _schema;
    std::vector<io_handler> _col_handlers;
    io_handler _count_handler;
public:
    ver_table(const std::string& table_path) {
        auto maybe_backslash = table_path[table_path.size() - 1] == '/' ? "" : "/";
        _schema = schema(table_path + maybe_backslash + DEFAULT_SCHEMA_NAME);
        _count_handler = io_handler(table_path + maybe_backslash +  _schema.get_name() + CNT_SUFF);

        auto col_num = _schema.col_num();
        _col_handlers.resize(col_num);
        for(int dim = 0; dim < col_num; ++dim)
            _col_handlers[dim] = io_handler(table_path + maybe_backslash +  _schema.get_name() + "/" + _schema.get_column(dim).name + VER_TABLE_SUFF);
    }

    // returns list of entries
    void read_cols(std::vector<int32_t>& buff, int dim, uint32_t cols_num, size_t col_offset = 0) {
        int32_t avail = count() - col_offset;
        
        if (avail <= 0) return;
        cols_num = std::min(int32_t(cols_num), avail);

        auto col_size = get_size(_schema.get_column(dim).type);
        _col_handlers[dim].read(buff, cols_num * col_size / 4, col_offset * col_size);
    }

    void read_rows(std::vector<row>& rows, uint32_t rows_num, size_t row_offset = 0) {
        int32_t avail = count() - row_offset;
        
        if (avail <= 0) return;

        rows_num = std::min(int32_t(rows_num), avail);
        rows.reserve(rows.size() + rows_num);

        for (size_t i = 0; i < rows_num; ++i) {
            std::vector<int32_t> row_data;
            for(size_t dim = 0; dim < _col_handlers.size(); ++dim)
                read_cols(row_data, dim, 1, row_offset);
            row_offset++;
            rows.emplace_back(row_data, _schema);
        }
    }

    uint32_t count() {
        return _count_handler.read_one();
    }

private:
    void increment_count() {
        uint32_t cnt = count() + 1;
        _count_handler.write_one(cnt);
    }

    void decrement_count() {
        uint32_t cnt = count() - 1;
        _count_handler.write_one(cnt);
    }
};