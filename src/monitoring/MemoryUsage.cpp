#include "MemoryUsage.h"

std::optional<MemoryUsage> calculateMemoryUsage(const MemoryInfo& memoryInfo) {
    if (memoryInfo.totalBytes == 0 || memoryInfo.availableBytes > memoryInfo.totalBytes) {
        return std::nullopt;
    }

    MemoryUsage memoryUsage;
    memoryUsage.usedBytes = memoryInfo.totalBytes - memoryInfo.availableBytes;
    memoryUsage.usagePercent = 100.0 * memoryUsage.usedBytes / memoryInfo.totalBytes;

    return memoryUsage;
}
