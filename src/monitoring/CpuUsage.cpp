#include "CpuUsage.h"

std::optional<double> calculateCpuUsage(const CpuTimes& previous, const CpuTimes& current) {
    if (current.idle < previous.idle || current.user < previous.user || current.kernel < previous.kernel) {
        return std::nullopt;
    }
    const std::uint64_t totalDelta = (current.kernel - previous.kernel) + (current.user - previous.user);
    if (totalDelta == 0) {
        return std::nullopt;
    }

    const std::uint64_t idleDelta = current.idle - previous.idle;
    if (idleDelta > totalDelta) {
        return std::nullopt;
    }

    const std::uint64_t busyDelta = totalDelta - idleDelta;
    return 100.0 * busyDelta / totalDelta;
}
