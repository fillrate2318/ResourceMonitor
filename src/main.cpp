#include <thread>
#include <chrono>
#include <iostream>
#include <system_error>

#include "monitoring/CpuTimes.h"
#include "monitoring/CpuUsage.h"
#include "monitoring/windows/CpuTimesReader.h"

void runMonitoring() {
    auto previous = readCpuTimes();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        try {
            const auto current = readCpuTimes();

            if (auto usage = calculateCpuUsage(previous, current)) {
                std::cout << "CPU usage: " << *usage << " %\n";
            }

            previous = current;
        } catch (const std::system_error& e) {
            std::cerr << "CPU sampling failed: " << e.what() << std::endl;
        }
    }
}

int main() {
    try {
        runMonitoring();
    } catch (const std::system_error& e) {
        std::cerr << "Monitoring failed to start: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
