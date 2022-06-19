#pragma once

#include <vector>
#include <cstdint>
#include "schema.h"

class row {
    std::vector<int32_t> _data;
    schema& _schema;
public:
    row(std::vector<int32_t>& data, schema& schema) : _data(std::move(data)), _schema(schema) {}
    std::vector<int32_t>& data() { return _data; }
};