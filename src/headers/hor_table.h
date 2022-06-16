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

    // TODO make more high level, ex. read nth row from file (in this case function would return a row, not use it as buffer)
    void read(std::vector<int32_t>& buffer, uint32_t size, uint32_t offset = 0) {
        _table_handler.read(buffer, size, offset);
    }

    void insert(row& new_row) {
        _table_handler.write(new_row.data());
        increment_count();  // TODO only if successful
    }

    uint32_t count() {
        return _count_handler.read_one();
    }
private:
    void increment_count() {
        uint32_t cnt = count() + 1;
        _count_handler.write_one(cnt, 0);
    }

    void decrement_count() {
        uint32_t cnt = count() - 1;
        _count_handler.write_one(cnt, 0);
    }
};