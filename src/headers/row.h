#pragma once

#include <vector>
#include <cstdint>
#include "schema.h"

class row {
    std::vector<int32_t> _data;
public:
    row() {}
    row(std::vector<int32_t>& data) : _data(data) {}
    row(std::initializer_list<int32_t> init) : _data(init) { }

    std::vector<int32_t>& data() { return _data; }
};