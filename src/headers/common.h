#pragma once

constexpr const char* DEFAULT_SCHEMA_NAME = "schema.txt";
constexpr const char* HOR_TABLE_SUFF = ".hor";
constexpr const char* VER_TABLE_SUFF = ".ver";
constexpr const char* CNT_SUFF = ".cnt";
constexpr const size_t BLOCK_SIZE = 512;

static std::string maybe_backslash(const std::string& in) {
    return in[in.size() - 1] == '/' ? "" : "/";
}