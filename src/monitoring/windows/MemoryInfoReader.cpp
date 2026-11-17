#include "MemoryInfoReader.h"

#include <system_error>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

MemoryInfo readMemoryInfo() {
    MemoryInfo memoryInfo;

    MEMORYSTATUSEX memoryStatus{};

    memoryStatus.dwLength = sizeof(memoryStatus);
    if (!GlobalMemoryStatusEx(&memoryStatus)) {
        throw std::system_error(GetLastError(), std::system_category(),
            "Failed to get memory status");
    }

    memoryInfo.totalBytes = memoryStatus.ullTotalPhys;
    memoryInfo.availableBytes = memoryStatus.ullAvailPhys;

    return memoryInfo;
}
