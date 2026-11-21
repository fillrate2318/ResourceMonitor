#include "CpuUsage.h"

#include <limits>

std::optional<double> calculateCpuUsage(const CpuTimes& previous, const CpuTimes& current) {
    if (current.idle < previous.idle || current.user < previous.user || current.kernel < previous.kernel) {
        return std::nullopt;
    }
    const std::uint64_t kernelDelta = current.kernel - previous.kernel;
    const std::uint64_t userDelta = current.user - previous.user;
    const std::uint64_t idleDelta = current.idle - previous.idle;
    if (idleDelta > kernelDelta ||
        userDelta > std::numeric_limits<std::uint64_t>::max() - kernelDelta) {
        return std::nullopt;
    }

    const std::uint64_t totalDelta = kernelDelta + userDelta;
    if (totalDelta == 0) {
        return std::nullopt;
    }

    const std::uint64_t busyDelta = totalDelta - idleDelta;
    return 100.0 * busyDelta / totalDelta;
}
