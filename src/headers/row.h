#pragma once

#include <vector>
#include <cstdint>
#include "schema.h"

class row {
    std::vector<int32_t> _data;
    schema& _schema;
public:
    row(std::vector<int32_t>& data, schema& schema) : _data(std::move(data)), _schema(schema) {
        assert(_data.size() == _schema.row_size() / 4);
    }

    std::vector<int32_t>& data() { return _data; }

    // debug function, not to be used
    void print_values() {
        for (size_t dim = 0; dim < _schema.col_num(); dim++)
        {
            auto type = _schema.get_column(dim).type;
            int32_t* ptr = _data.data() + _schema.offset(dim);
            auto val = get_value(ptr, type);
            std::cout << val << "\n";
        }
        
    }
};