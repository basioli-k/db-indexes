#pragma once
#include <assert.h>
#include <numeric>
#include <string>
#include <vector>
#include <fstream>
#include <exception>
#include "custom_types.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

class schema {
    struct column {
        types type;
        std::string name;
    };
    std::string _tname;
    std::vector<column> _cols; 
public:
    schema() {} // default constructor
    schema(std::string schema_path) {
        std::ifstream schema_file(schema_path);
        assert(schema_file.is_open());
        
        getline(schema_file, _tname);
        
        std::string schema_line;
        getline(schema_file, schema_line);
        
        std::vector<std::string> col_types;
        boost::algorithm::split(col_types, schema_line, boost::algorithm::is_any_of(";"));
        
        _cols.reserve(col_types.size());
        for (auto& col_type : col_types) {
            std::vector<std::string> col;
            col.reserve(2);
            boost::algorithm::split(col, col_type, boost::algorithm::is_any_of(":"));            
            assert( col.size() == 2 );
            _cols.emplace_back( get_type(col[0]), col[1] );
        }
    }

    const std::string& get_name() { return _tname; }

    const column& get_column(int dim) { return _cols[dim]; }

    std::size_t col_num() { return _cols.size(); }

    // returns offset in _data array of the row
    int offset(int dim) {
        assert(dim < _cols.size());
        int offset = 0;
        for (int i = 0 ; i < dim; ++i)
            offset += get_size(_cols[i].type);

        return offset / 4;
    }

    // return row_size in bytes
    // WARNING row size returns number of bytes, row has a vector of 4 byte ints
    std::size_t row_size() { 
        auto sum_col = [](std::size_t acc, column& col) { return acc + get_size(col.type); };
        return std::accumulate(_cols.begin(), _cols.end(), std::size_t(0), sum_col);
    }
};
