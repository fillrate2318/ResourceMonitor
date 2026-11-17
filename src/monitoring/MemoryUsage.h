#pragma once

#include <cstdint>
#include <optional>

#include "MemoryInfo.h"

struct MemoryUsage {
    std::uint64_t usedBytes = 0;
    double usagePercent = 0.0;
};

std::optional<MemoryUsage> calculateMemoryUsage(const MemoryInfo& memoryInfo);