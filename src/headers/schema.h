#pragma once
#include <assert.h>
#include <string>
#include <vector>
#include <fstream>
#include <exception>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

// TODO remove
#include <iostream>
enum class types {
    int32_t,
    uint32_t,
    int64_t,
    uint64_t,
    float32,
    float64
};

types get_type(const std::string& str_type) {
    if (str_type == "int32_t")
        return types::int32_t;
    else if (str_type == "uint32_t")
        return types::uint32_t;
    else if (str_type == "int64_t")
        return types::int64_t;
    else if (str_type == "uint64_t")
        return types::uint64_t;
    else if (str_type == "float")
        return types::float32;
    else if (str_type == "double")
        return types::float64;
    else
        throw std::invalid_argument("Unsupported type.");
}

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
};
