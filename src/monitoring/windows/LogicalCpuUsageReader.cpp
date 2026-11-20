#include "LogicalCpuUsageReader.h"

#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <system_error>
#include <string_view>
#include <algorithm>
#include <utility>

struct LogicalCpuUsageReader::Impl {
    PDH_HQUERY query = nullptr;
    PDH_HCOUNTER counter = nullptr;
    bool hasPreviousSample = false;

    ~Impl() {
        if (query) {
            PdhCloseQuery(query);
        }
    }
};

LogicalCpuUsageReader::LogicalCpuUsageReader() : impl_(std::make_unique<Impl>()) {
    openQuery();
}

LogicalCpuUsageReader::~LogicalCpuUsageReader() = default;

std::optional<std::vector<LogicalCpuUsage>> LogicalCpuUsageReader::sample(std::optional<double>& total) {
    auto status = PdhCollectQueryData(impl_->query);
    if (status != ERROR_SUCCESS) {
        throw std::system_error(status, std::system_category(),
            "Failed to collect processor time sample");
    }

    if (!impl_->hasPreviousSample) {
        impl_->hasPreviousSample = true;
        return std::nullopt;
    }

    DWORD bufferSize = 0;
    DWORD itemsCount = 0;

    status = PdhGetFormattedCounterArrayW(impl_->counter, PDH_FMT_DOUBLE, &bufferSize, &itemsCount, nullptr);
    if (status != PDH_MORE_DATA) {
        throw std::system_error(status, std::system_category(),
            "Failed to determine PDH counter array size");
    }

    auto buffer = std::make_unique<BYTE[]>(bufferSize);
    auto* items = reinterpret_cast<PPDH_FMT_COUNTERVALUE_ITEM_W>(buffer.get());

    status = PdhGetFormattedCounterArrayW(impl_->counter, PDH_FMT_DOUBLE, &bufferSize, &itemsCount, items);
    if (status != ERROR_SUCCESS) {
        throw std::system_error(status, std::system_category(),
            "Failed to read PDH counter array");
    }

    std::vector<LogicalCpuUsage> result;
    result.reserve(itemsCount);

    for (DWORD i = 0; i < itemsCount; ++i) {
        const auto& item = items[i];

        const std::wstring_view name{item.szName};

        if (name.ends_with(L",_Total")) {
            continue;
        }

        std::optional<double> value = std::nullopt;
        if (item.FmtValue.CStatus == PDH_CSTATUS_VALID_DATA || item.FmtValue.CStatus == PDH_CSTATUS_NEW_DATA) {
            value = item.FmtValue.doubleValue;
        }

        if (name == L"_Total") {
            total = value;
            continue;
        }

        int group, number;
        if (swscanf_s(item.szName, L"%d , %d", &group, &number) == 2) {
            result.push_back({group, number, value});
        }

    }

    std::ranges::sort(result, {}, [](const auto& cpu) {
        return std::pair{cpu.group, cpu.number};
    });
    return result;
}

void LogicalCpuUsageReader::openQuery() {
    auto status = PdhOpenQueryW(nullptr, 0, &impl_->query);
    if (status != ERROR_SUCCESS) {
        throw std::system_error(status, std::system_category(), "Failed to open PDH query");
    }

    status = PdhAddEnglishCounterW(
        impl_->query,
        LR"(\Processor Information(*)\% Processor Time)",
        0,
        &impl_->counter
    );

    if (status != ERROR_SUCCESS) {
        throw std::system_error(status, std::system_category(),
            "Failed to add processor time counter");
    }
}
