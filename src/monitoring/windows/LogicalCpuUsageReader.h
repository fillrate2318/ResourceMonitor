#pragma once

#include <memory>
#include <optional>
#include <vector>

struct LogicalCpuUsage {
    int group = 0;
    int number = 0;
    std::optional<double> usagePercent;
};

struct LogicalCpuUsageSample {
    std::optional<double> totalUsagePercent;
    std::vector<LogicalCpuUsage> processors;
};

class LogicalCpuUsageReader {
public:
    LogicalCpuUsageReader();
    LogicalCpuUsageReader(const LogicalCpuUsageReader&) = delete;
    ~LogicalCpuUsageReader();

    LogicalCpuUsageReader& operator=(const LogicalCpuUsageReader&) = delete;

    std::optional<LogicalCpuUsageSample> sample();

private:
    void openQuery();

    struct Impl;
    std::unique_ptr<Impl> impl_;
};
