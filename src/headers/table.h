#pragma once
#include "row.h"

class table {
protected:
    schema _schema;
public:
    table(const std::string& table_path) {
        auto maybe_backslash = table_path[table_path.size() - 1] == '/' ? "" : "/";
        _schema = schema(table_path + maybe_backslash + DEFAULT_SCHEMA_NAME);
    }
    virtual void read_rows(std::vector<row>& rows, uint32_t rows_num, size_t row_offset = 0) = 0;
    virtual void insert(row& new_row) = 0;
    virtual int32_t count() = 0;
};