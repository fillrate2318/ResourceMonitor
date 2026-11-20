#include "CpuFrequencyReader.h"

#include <windows.h>
#include <powrprof.h>
#include <iostream>
#include <string>
#include <system_error>

typedef struct _PROCESSOR_POWER_INFORMATION {
    ULONG Number;
    ULONG MaxMhz;
    ULONG CurrentMhz;
    ULONG MhzLimit;
    ULONG MaxIdleState;
    ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

std::vector<int> readCpuFrequencies() {
    SYSTEM_INFO systemInfo{};
    GetSystemInfo(&systemInfo);

    DWORD numberOfProcessors = systemInfo.dwNumberOfProcessors;
    std::vector<PROCESSOR_POWER_INFORMATION> processors(numberOfProcessors);

    const NTSTATUS status = CallNtPowerInformation(
        ProcessorInformation,
        nullptr,
        0,
        processors.data(),
        numberOfProcessors * sizeof(PROCESSOR_POWER_INFORMATION));

    if (status != ERROR_SUCCESS) {
        throw std::system_error(status, std::system_category(),
            "Power information call failed");
    }

    std::vector<int> result;
    result.reserve(numberOfProcessors);

    for (const auto& processor : processors) {
        result.push_back(processor.CurrentMhz);
    }

    return result;
}