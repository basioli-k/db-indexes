#pragma once
#include <chrono>

class stopwatch {
    std::chrono::steady_clock::time_point _start;
public:
    stopwatch() {}

    void start() {
        _start = std::chrono::steady_clock::now();
    }

    // returns duration in nanoseconds
    long long stop() {
        return (std::chrono::steady_clock::now() - _start).count();
    }
};