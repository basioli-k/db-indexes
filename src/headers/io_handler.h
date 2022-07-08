#pragma once

#include <vector>
#include <fstream>

class io_handler{
    std::fstream _file;
public:
    io_handler() {}
    io_handler(const std::string& file_path) : _file(file_path, std::ios::binary | std::ios::in | std::ios::out) { }

    void seekg(size_t offset) {
        _file.seekg(offset);
    }

    void seekg_to_end() {
        _file.seekg(0, std::ios::end);
    }

    // reads size bytes at and stores the into buffer
    void read(std::vector<int32_t>& buffer, size_t size) {
        buffer.resize(buffer.size() + size);
        _file.read(reinterpret_cast<char*>(buffer.data() + buffer.size() - size), size * sizeof(int32_t));
    }

    int32_t read_one() {
        int32_t count = 0;
        _file.read(reinterpret_cast<char*>(&count), sizeof(int32_t));
        return count;
    }

    void write(std::vector<int32_t>& buffer) {
        _file.write(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(int32_t));
    }

    void write(std::vector<int32_t>::iterator it, size_t size) {
        _file.write(reinterpret_cast<char*>(&(*it)), size * sizeof(int32_t));
    }
    
    void write_one(int32_t element) {
        _file.write(reinterpret_cast<char*>(&element), sizeof(int32_t));
    }
};