#pragma once

#include <memory>
#include <optional>
#include <vector>

struct LogicalCpuUsage {
    int group = 0;
    int number = 0;
    std::optional<double> usagePercent;
};

class LogicalCpuUsageReader {

public:
    LogicalCpuUsageReader();
    LogicalCpuUsageReader(const LogicalCpuUsageReader&) = delete;
    ~LogicalCpuUsageReader();

    LogicalCpuUsageReader& operator=(const LogicalCpuUsageReader&) = delete;

    std::optional<std::vector<LogicalCpuUsage>> sample(std::optional<double>& total);

private:
    void openQuery();

    struct Impl;
    std::unique_ptr<Impl> impl_;
};
