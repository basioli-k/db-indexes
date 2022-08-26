#pragma once

constexpr const char* DEFAULT_SCHEMA_NAME = "schema.txt";
constexpr const char* HOR_TABLE_SUFF = ".hor";
constexpr const char* VER_TABLE_SUFF = ".ver";
constexpr const char* CNT_SUFF = ".cnt";
constexpr const char* BTREE_SUFF = ".btree";
constexpr const char* META_SUFF = ".meta";
constexpr const char* HASH_SUFF = ".hash";

constexpr const size_t BLOCK_SIZE = 512;
constexpr size_t TREE_NODE_HDR_LEN = 2;
constexpr size_t NODE_PARAM = (BLOCK_SIZE - 4 * TREE_NODE_HDR_LEN - 4) / 8; // TODO last two constants assume four byte sized key
constexpr size_t HASH_PARAM = (BLOCK_SIZE - 4 * 2) / (4 * 2); // TODO assuming four byte sized key

static std::string maybe_backslash(const std::string& in) {
    return in[in.size() - 1] == '/' ? "" : "/";
}