#pragma once

#include <cstdint>

struct MemoryInfo {
    std::uint64_t totalBytes = 0;
    std::uint64_t availableBytes = 0;
};
