#pragma once

#include <cstdint>

struct CpuTopology {
    std::uint32_t physicalCoreCount = 0;
    std::uint32_t logicalProcessorCount = 0;
};