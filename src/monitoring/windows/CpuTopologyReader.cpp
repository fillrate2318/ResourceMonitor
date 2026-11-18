#include "CpuTopologyReader.h"

#include <bit>
#include <memory>
#include <system_error>
#include <cstddef>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

CpuTopology readCpuTopology() {
    DWORD length = 0;
    if (!GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &length)) {
        const DWORD error = GetLastError();
        if (error != ERROR_INSUFFICIENT_BUFFER) {
            throw std::system_error(error, std::system_category(),
                "Failed to read CPU topology");
        }
    }

    const auto buffer = std::make_unique<BYTE[]>(length);
    auto* info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.get());
    if (!GetLogicalProcessorInformationEx(RelationProcessorCore, info, &length)) {
        throw std::system_error(GetLastError(), std::system_category(),
            "Failed to read CPU topology");
    }

    DWORD offset = 0;
    CpuTopology cpuTopology;

    using ProcessorInfo = SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX;

    constexpr size_t headerSize =
        offsetof(ProcessorInfo, Size) + sizeof(DWORD);

    constexpr size_t minCoreRecordSize =
        offsetof(ProcessorInfo, Processor) + offsetof(PROCESSOR_RELATIONSHIP, GroupMask) + sizeof(GROUP_AFFINITY);

    while (offset < length) {
        const size_t remaining = length - offset;

        if (remaining < headerSize) {
            throw std::runtime_error("Invalid processor info header");
        }

        auto* element = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.get() + offset);

        if (element->Size > remaining) {
            throw std::runtime_error("Invalid processor info size");
        }

        if (element->Relationship != RelationProcessorCore) {
            throw std::runtime_error("Unexpected processor relationship");
        }

        if (element->Size < minCoreRecordSize) {
            throw std::runtime_error("Invalid processor info size");
        }

        ++cpuTopology.physicalCoreCount;
        cpuTopology.logicalProcessorCount += std::popcount(element->Processor.GroupMask->Mask);

        offset += element->Size;
    }

    return cpuTopology;
}