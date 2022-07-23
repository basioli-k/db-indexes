#pragma once
#include "common.h"
#include "row.h"

class table {
protected:
    schema _schema;
private:
    io_handler _count_handler;
    int32_t _count;
public:
    table(const std::string& table_path) : _schema(table_path + maybe_backslash(table_path) + DEFAULT_SCHEMA_NAME),
        _count_handler(table_path + maybe_backslash(table_path) +  _schema.get_name() + CNT_SUFF) 
    {
        auto maybe_backslash = table_path[table_path.size() - 1] == '/' ? "" : "/";
        update_count();
    }

    virtual void read_rows(std::vector<row>& rows, int32_t rows_num, size_t row_offset = 0) = 0;
    virtual void insert(row& new_row) = 0;

    int32_t count() {
        return _count;
    }
protected:
    void update_count() {
        _count_handler.seekg(0);
        std::vector<int32_t> buff;
        _count_handler.read(buff, 1);
        _count = buff[0];
    }
    void increment_count() {
        int32_t cnt = _count + 1;
        _count_handler.seekg(0);
        // _count_handler.write_one(cnt);
        update_count();
    }

    void decrement_count() {
        int32_t cnt = _count - 1;
        _count_handler.seekg(0);
        // _count_handler.write_one(cnt);
        update_count();
    }
};