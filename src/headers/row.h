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

    // auto get_val(int dim) {
    //     int offset = _schema.offset(dim);
    //     int32_t* ptr = _data.data() + offset;
    //     switch(_schema.get_column(dim).type) {
    //         case types::int32_t:
    //             return *(reinterpret_cast<int32_t*>(ptr));
    //         case types::uint32_t:
    //             return *(reinterpret_cast<uint32_t*>(ptr));;
    //         case types::int64_t:
    //             return *(reinterpret_cast<int64_t*>(ptr));;
    //         case types::uint64_t:
    //             return *(reinterpret_cast<uint64_t*>(ptr));;
    //         case types::float32:
    //             return *(reinterpret_cast<float*>(ptr));;
    //         case types::float64:
    //             return *(reinterpret_cast<double*>(ptr));;
    //         default:
    //             throw std::invalid_argument("Unsupported type.");
    //     }
    // }
};