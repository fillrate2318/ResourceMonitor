#pragma once

#include <string>
#include <optional>
#include <vector>
#include <pdh.h>

struct LogicalCpuUsage {
    std::wstring processorId;
    std::optional<double> usagePercent;
};

class LogicalCpuUsageReader {

public:
    LogicalCpuUsageReader();
    LogicalCpuUsageReader(const LogicalCpuUsageReader&) = delete;
    ~LogicalCpuUsageReader();

    LogicalCpuUsageReader& operator=(const LogicalCpuUsageReader&) = delete;

    std::optional<std::vector<LogicalCpuUsage>> sample();

private:
    void openQuery();

    PDH_HQUERY query_ = nullptr;
    PDH_HCOUNTER counter_ = nullptr;

    bool hasPreviousSample_ = false;
};
