#include "LogicalCpuUsageReader.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <string_view>
#include <system_error>
#include <utility>

#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>

namespace {
    std::optional<int> parseProcessorIndex(std::wstring_view text) {
        if (text.empty()) {
            return std::nullopt;
        }

        int result = 0;
        for (const wchar_t character : text) {
            if (character < L'0' || character > L'9') {
                return std::nullopt;
            }
            const int digit = character - L'0';
            if (result > (INT_MAX - digit) / 10) {
                return std::nullopt;
            }
            result = result * 10 + digit;
        }
        return result;
    }
}

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

std::optional<LogicalCpuUsageSample> LogicalCpuUsageReader::sample() {
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
    if (status != static_cast<PDH_STATUS>(PDH_MORE_DATA)) {
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

    LogicalCpuUsageSample result;
    result.processors.reserve(itemsCount);

    for (DWORD i = 0; i < itemsCount; ++i) {
        const auto& item = items[i];

        const std::wstring_view name{item.szName};

        std::optional<double> value;
        if (item.FmtValue.CStatus == PDH_CSTATUS_VALID_DATA || item.FmtValue.CStatus == PDH_CSTATUS_NEW_DATA) {
            const auto percent = item.FmtValue.doubleValue;
            if (std::isfinite(percent) && percent >= 0.0 && percent <= 100.0) {
                value = percent;
            }
        }

        if (name == L"_Total") {
            result.totalUsagePercent = value;
            continue;
        }

        const auto separator = name.find(L',');
        if (separator == std::wstring_view::npos) {
            continue;
        }
        const auto group = parseProcessorIndex(name.substr(0, separator));
        const auto number = parseProcessorIndex(name.substr(separator + 1));
        if (group && number) {
            result.processors.push_back({*group, *number, value});
        }
    }

    std::ranges::sort(result.processors, {}, [](const auto& cpu) {
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
