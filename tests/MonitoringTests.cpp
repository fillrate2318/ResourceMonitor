#include <cmath>
#include <iostream>
#include <optional>

#include "monitoring/CpuUsage.h"

namespace {
    struct CpuUsageTestCase {
        const char* name;
        CpuTimes previous;
        CpuTimes current;
        std::optional<double> expected;
    };

    bool testCpuUsage(const CpuUsageTestCase& testCase) {
        const auto actual = calculateCpuUsage(testCase.previous, testCase.current);

        if (!testCase.expected) {
            if (!actual) {
                return true;
            }
            std::cerr << "FAIL: " << testCase.name << ": expected nullopt, got " << *actual << "\n";
            return false;
        }

        if (!actual) {
            std::cerr << "FAIL: " << testCase.name << ": expected " << *testCase.expected << ", got nullopt\n";
            return false;
        }

        constexpr double tolerance = 1e-6;
        if (!std::isfinite(*actual) ||
            std::abs(*actual - *testCase.expected) > tolerance) {
            std::cerr << "FAIL: " << testCase.name << ": expected " << *testCase.expected << ", got " << *actual << "\n";
            return false;
            }

        return true;
    }

    constexpr CpuUsageTestCase cpuUsageCases[] = {
        {"75% load", {100, 200, 300}, {125, 280, 320}, 75.0},
        {"Fully idle", {100, 200, 300}, {200, 300, 300}, 0.0},
        {"Fully busy", {100, 200, 300}, {100, 280, 320}, 100.0},
        {"Zero interval", {100, 200, 300}, {100, 200, 300}, std::nullopt},
        {"Decreasing idle", {100, 200, 300}, {0, 200, 300}, std::nullopt},
        {"Decreasing kernel", {100, 200, 300}, {100, 0, 300}, std::nullopt},
        {"Decreasing user", {100, 200, 300}, {100, 200, 0}, std::nullopt},
        {"Invalid idle delta", {100, 200, 300}, {400, 300, 400}, std::nullopt},
        {"Unchanged user", {100, 200, 300}, {125, 300, 300}, 75.0},
    };
}

int main() {
    int failed = 0;

    for (const auto& testCase : cpuUsageCases) {
        if (!testCpuUsage(testCase)) {
            failed++;
        }
    }

    if (failed != 0) {
        std::cerr << failed << " test(s) failed\n";
        return 1;
    }

    std::cout << "All tests passed\n";
    return 0;
}
