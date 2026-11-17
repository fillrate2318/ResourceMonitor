#include "CpuTimesReader.h"

#include <system_error>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace {
    std::uint64_t fileTimeToUint64(const FILETIME& fileTime) {
        return (static_cast<std::uint64_t>(fileTime.dwHighDateTime) << 32) |
            static_cast<std::uint64_t>(fileTime.dwLowDateTime);
    }
}

CpuTimes readCpuTimes() {
    CpuTimes cpuTimes;

    FILETIME idleTime, kernelTime, userTime;

    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        throw std::system_error(GetLastError(), std::system_category(),
            "Failed to get system times");
    }
    cpuTimes.idle = fileTimeToUint64(idleTime);
    cpuTimes.kernel = fileTimeToUint64(kernelTime);
    cpuTimes.user = fileTimeToUint64(userTime);

    return cpuTimes;
}
