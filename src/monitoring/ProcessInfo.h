#pragma once

#include <cstdint>
#include <string>

struct ProcessInfo {
    std::uint32_t pid;
    std::wstring name;
    std::uint32_t threadCount;
};
