#pragma once
#include <fstream>
#include <string>
#include <vector>
#include "row.h"
#include "schema.h"

// TODO should move this to some header with 
constexpr const char* DEFAULT_SCHEMA_NAME = "schema.txt";

constexpr const char* HOR_TABLE_SUFF = ".hor";

class hor_table {
    schema _schema;
    std::ifstream _file;    // smeta li ikome ako se ovo closea tek kad se hor table destructa?
public: 
    hor_table(const std::string& table_path) {
        auto maybe_backslash = table_path[table_path.size() - 1] == '/' ? "" : "/";
        _schema = schema(table_path + maybe_backslash +  DEFAULT_SCHEMA_NAME);
        
        _file.open(table_path + maybe_backslash + _schema.get_name() + HOR_TABLE_SUFF, std::ios::binary | std::ios::in);
    }

    // reads size bytes at offset and stores the into buffer
    void read(std::vector<int32_t>& buffer, uint32_t size, uint32_t offset = 0) {
        // TODO napraviti kontrolu da nikad ne izletimo van filea (odnosno offset + size * sizeof(int32_t) bi trebalo biti <= file size tako nesto)
        buffer.resize(buffer.size() + size);
        _file.read(reinterpret_cast<char*>(buffer.data() + buffer.size() - size), size * sizeof(int32_t));

        // seekg opcija za offset zvuci obecavajuce
    }
    // write
    // update
};