#include <cmath>
#include <iostream>
#include <optional>

#include "monitoring/CpuUsage.h"
#include "monitoring/MemoryUsage.h"

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
        {"Decreasing idle", {100, 200, 300}, {99, 280, 320}, std::nullopt},
        {"Decreasing kernel", {100, 200, 300}, {125, 199, 320}, std::nullopt},
        {"Decreasing user", {100, 200, 300}, {125, 280, 299}, std::nullopt},
        {"Invalid idle delta", {100, 200, 300}, {400, 300, 400}, std::nullopt},
        {"Unchanged user", {100, 200, 300}, {125, 300, 300}, 75.0},
        {"Fractional load", {100, 200, 300}, {102, 202, 301}, 100.0 / 3.0},
    };
}

namespace {
    struct MemoryUsageTestCase {
        const char* name;
        MemoryInfo memoryInfo;
        std::optional<MemoryUsage> expected;
    };

    std::ostream& operator<<(std::ostream& os, const MemoryUsage& usage) {
        return os << "{usedBytes: " << usage.usedBytes << ", usagePercent: " << usage.usagePercent << "}";
    }

    bool testMemoryUsage(const MemoryUsageTestCase& testCase) {
        const auto actual = calculateMemoryUsage(testCase.memoryInfo);

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
        if (!std::isfinite(actual->usagePercent) ||
                std::abs(actual->usagePercent - testCase.expected->usagePercent) > tolerance ||
                actual->usedBytes != testCase.expected->usedBytes) {
            std::cerr << "FAIL: " << testCase.name << ": expected " << *testCase.expected << ", got " <<
                *actual << "\n";
            return false;
        }

        return true;
    }

    constexpr MemoryUsageTestCase memoryUsageCases[] = {
        {"75% load", {1000, 250}, MemoryUsage{750, 75.0}},
        {"Zero load", {1000, 1000}, MemoryUsage{0, 0.0}},
        {"Fully loaded", {1000, 0}, MemoryUsage{1000, 100.0}},
        {"Zero total", {0, 0}, std::nullopt},
        {"Invalid available", {1000, 2000}, std::nullopt},
        {"Floating percent", {300, 100}, MemoryUsage{200, 200.0 / 3.0}}
    };
}

int main() {
    int failed = 0;

    for (const auto& testCase : cpuUsageCases) {
        if (!testCpuUsage(testCase)) {
            failed++;
        }
    }

    for (const auto& testCase : memoryUsageCases) {
        if (!testMemoryUsage(testCase)) {
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
