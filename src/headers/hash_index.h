#pragma once
#include <cstdint>
#include <filesystem>
#include <vector>
#include <string>
#include <memory>
#include "io_handler.h"

namespace hash {

class metadata {
    int32_t _max_bid; // max block id
    std::unique_ptr<io_handler> _file;
    const std::string _path;
public:
    metadata(const std::string& path) : _path(path), _file(nullptr) {}

        void load() {
        if (!_file)
            open_md();
        std::vector<int32_t> buff;
        _file->read(buff, 1);
        _max_bid = buff[0];
    }

    void update_md(int32_t max_bid) {
        if (!_file)
            open_md();
        _max_bid = max_bid;
        std::vector<int32_t> buff{ _max_bid };
        _file->seekg(0);
        _file->write(buff);
    }

    int32_t max_bid() { return _max_bid; }
private:
    void open_md() {
        _file = std::make_unique<io_handler>(_path);
    }
};

}; // end namespace hash

class bucket_block {
    static std::string path;
    hash::metadata& _meta;
    int32_t _count;
    int32_t _next;  
    std::vector<int32_t> _keys;
    std::vector<int32_t> _offsets;
    int32_t _bid; // TODO remove, not needed
    std::unique_ptr<io_handler> _file;
public:
    bucket_block(int32_t bid, hash::metadata& meta) : _meta(meta), _bid(bid) {
        auto file_path = path + maybe_backslash(path) + std::to_string(bid) + HASH_SUFF;

        if(!file_exists(file_path)) {
            _file = std::make_unique<io_handler>(file_path);
            _count = 0;
            _next = -1;
            update_block();
            return;
        }
        _file = std::make_unique<io_handler>(file_path);

        std::vector<int32_t> buff;
        _file->read(buff, BLOCK_SIZE / 4);
        _count = buff[0];
        _next = buff[1];

        _keys.reserve(_count);
        _offsets.reserve(_count);
        for ( size_t i = 0; i < _count; ++i ) {
            _keys.push_back(buff[HASH_BLOCK_HDR_LEN / 4 + i]);
            _offsets.push_back(buff[HASH_BLOCK_HDR_LEN / 4 + _count + i]);
        }
    }

    friend class hash_index;
private:
    void insert(int32_t val, int32_t offset) {
        assert(std::find(_keys.begin(), _keys.begin() + _count, val) == _keys.begin() + _count);    // can't have duplicate keys
        if (_count < ENTRIES_PER_BLOCK) {
            _keys.push_back(val);
            _offsets.push_back(offset);
            _count++;
            update_block();
            return;
        }

        if( _next == -1 ) { // continue chain if needed
            _meta.update_md(_meta.max_bid() + 1);
            _next = _meta.max_bid();
            std::cout << "LOG: chain to " << _next << "\n";
            update_block();
        }

        bucket_block next(_next, _meta);
        next.insert(val, offset);
    }

    int32_t search(int32_t val) {
        size_t i = 0;
        for (; i < _count ; ++i)
            if (_keys[i] == val) 
                break;

        if ( i != _count)
            return  _offsets[i];

        if (_next == -1)
            return -1;

        bucket_block next(_next, _meta);

        return next.search(val);
    }

    void update_block() {
        std::vector<int32_t> buff { _count, _next };
        std::copy(_keys.begin(), _keys.begin() + _count, std::back_inserter(buff));
        std::copy(_offsets.begin(), _offsets.begin() + _count, std::back_inserter(buff));

        _file->seekg(0);
        _file->write(buff);
    }
};

class hash_index {
private:
    int32_t B; // number of blocks
    hash::metadata _meta;

public:
    hash_index(int32_t num_blocks) : B(num_blocks), _meta(bucket_block::path + maybe_backslash(bucket_block::path) + META_SUFF) {
        auto md_path = bucket_block::path + maybe_backslash(bucket_block::path) + META_SUFF;

        if (file_exists(md_path)) { // root already exists
            _meta.load();
        }
        else { // hash_index doesn't exist create it
            create_hash_folder();
            _meta.update_md(B - 1); // we are assuming that all bids from 0 to B - 1 are taken, chains will have values above or equal to B
        }
    }

    void insert(int32_t val, int32_t offset) {
        bucket_block bl(hash_int32(val), _meta);
        bl.insert(val, offset);
    }

    int32_t search(int32_t val) {
        bucket_block bl(hash_int32(val), _meta);
        return bl.search(val);
    }

private:
    int32_t hash_int32(int32_t val) {
        int32_t ret = val % B;
        while (ret < 0 ) ret += B;
        return ret;
    }

    void create_hash_folder() {
        std::filesystem::create_directories(bucket_block::path);
    }
};

static hash_index create_hash_index(hor_table& table, size_t col_index) {
    hash_index hind((table.count() / ENTRIES_PER_BLOCK) + 1);
    query q(nullptr, query_type::star, 0); // select all

    auto res = table.execute_query(q);

    for(size_t i = 0 ; i < res.size(); ++i) {
        auto val = res.rows()[i].get_val(col_index);
        hind.insert(int32_t(val), i);  // TODO works for int32_t
    }

    return hind;
}