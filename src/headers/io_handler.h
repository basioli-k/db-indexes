#pragma once

#include <vector>
#include <fstream>

class io_handler{
    std::fstream _file;    // smeta li ikome ako se ovo closea tek kad se io handler destructa?
public:
    io_handler() {}
    io_handler(const std::string& file_path) : _file(file_path, std::ios::binary | std::ios::in | std::ios::out) { }

    // reads size bytes at offset and stores the into buffer
    void read(std::vector<int32_t>& buffer, size_t size, size_t offset = 0) {
        // TODO how do we now when we're done?
        buffer.resize(buffer.size() + size);
        _file.seekg(offset);
        _file.read(reinterpret_cast<char*>(buffer.data() + buffer.size() - size), size * sizeof(int32_t));
    }

    int32_t read_one(size_t offset = 0) {
        _file.seekg(offset);
        int32_t count = 0;
        _file.read(reinterpret_cast<char*>(&count), sizeof(int32_t));
        return count;
    }

    void write(std::vector<int32_t>& buffer, size_t offset) {
        _file.seekg(offset);
        _file.write(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(int32_t));
    }

    void write(std::vector<int32_t>& buffer) {
        _file.seekg(0, std::ios::end);
        _file.write(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(int32_t));
    }

    void write(std::vector<int32_t>::iterator it, size_t size) {
        _file.seekg(0, std::ios::end);
        _file.write(reinterpret_cast<char*>(&(*it)), size * sizeof(int32_t));
    }

    void write_one(int32_t element, size_t offset) {
        _file.seekg(offset);
        _file.write(reinterpret_cast<char*>(&element), sizeof(int32_t));
    }
    
    void write_one(int32_t element) {
        _file.seekg(0, std::ios::end);
        _file.write(reinterpret_cast<char*>(&element), sizeof(int32_t));
    }
};