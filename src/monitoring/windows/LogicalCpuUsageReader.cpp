#include "LogicalCpuUsageReader.h"

#include <pdhmsg.h>
#include <system_error>
#include <memory>
#include <string_view>

LogicalCpuUsageReader::LogicalCpuUsageReader() {
    openQuery();
}

LogicalCpuUsageReader::~LogicalCpuUsageReader() {
    if (query_) {
        PdhCloseQuery(query_);
    }
}

std::optional<std::vector<LogicalCpuUsage>> LogicalCpuUsageReader::sample() {
    auto status = PdhCollectQueryData(query_);
    if (status != ERROR_SUCCESS) {
        throw std::system_error(status, std::system_category(),
            "Failed to collect processor time sample");
    }

    if (!hasPreviousSample_) {
        hasPreviousSample_ = true;
        return std::nullopt;
    }

    DWORD bufferSize = 0;
    DWORD itemsCount = 0;

    status = PdhGetFormattedCounterArrayW(counter_, PDH_FMT_DOUBLE, &bufferSize, &itemsCount, nullptr);
    if (status != PDH_MORE_DATA) {
        throw std::system_error(status, std::system_category(),
            "Failed to determine PDH counter array size");
    }

    auto buffer = std::make_unique<BYTE[]>(bufferSize);
    auto* items = reinterpret_cast<PPDH_FMT_COUNTERVALUE_ITEM_W>(buffer.get());

    status = PdhGetFormattedCounterArrayW(counter_, PDH_FMT_DOUBLE, &bufferSize, &itemsCount, items);
    if (status != ERROR_SUCCESS) {
        throw std::system_error(status, std::system_category(),
            "Failed to read PDH counter array");
    }

    std::vector<LogicalCpuUsage> result;
    result.reserve(itemsCount);

    for (DWORD i = 0; i < itemsCount; ++i) {
        const auto& item = items[i];

        const std::wstring_view name{item.szName};

        if (name == L"_Total") {
            continue;
        }

        if (name.ends_with(L",_Total")) {
            continue;
        }

        std::optional<double> value = std::nullopt;
        if (item.FmtValue.CStatus == PDH_CSTATUS_VALID_DATA || item.FmtValue.CStatus == PDH_CSTATUS_NEW_DATA) {
            value = item.FmtValue.doubleValue;
        }

        result.push_back({std::wstring{name}, value});
    }
    return result;
}

void LogicalCpuUsageReader::openQuery() {
    auto status = PdhOpenQueryW(nullptr, 0, &query_);
    if (status != ERROR_SUCCESS) {
        throw std::system_error(status, std::system_category(), "Failed to open PDH query");
    }

    status = PdhAddEnglishCounterW(
        query_,
        LR"(\Processor Information(*)\% Processor Time)",
        0,
        &counter_
    );

    if (status != ERROR_SUCCESS)
    {
        PdhCloseQuery(query_);
        query_ = nullptr;

        throw std::system_error(status, std::system_category(),
            "Failed to add processor time counter");
    }
}
