#include "CpuFrequencyReader.h"

#include <windows.h>
#include <powrprof.h>
#include <system_error>

namespace {
    // ProcessorInformation uses this layout for each returned processor.
    struct ProcessorPowerInformation {
        ULONG number;
        ULONG maxMhz;
        ULONG currentMhz;
        ULONG mhzLimit;
        ULONG maxIdleState;
        ULONG currentIdleState;
    };
}

std::vector<std::uint32_t> readCpuFrequencies() {
    const DWORD processorCount = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
    if (processorCount == 0) {
        return {};
    }
    std::vector<ProcessorPowerInformation> processors(processorCount);

    const NTSTATUS status = CallNtPowerInformation(
        ProcessorInformation,
        nullptr,
        0,
        processors.data(),
        static_cast<ULONG>(processors.size() * sizeof(ProcessorPowerInformation)));

    if (status != ERROR_SUCCESS) {
        throw std::system_error(status, std::system_category(),
            "Power information call failed");
    }

    std::vector<std::uint32_t> result;
    result.reserve(processors.size());

    for (const auto& processor : processors) {
        if (processor.currentMhz != 0) {
            result.push_back(processor.currentMhz);
        }
    }

    return result;
}
