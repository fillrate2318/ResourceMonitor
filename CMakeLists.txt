cmake_minimum_required(VERSION 4.2)
project(ResourceMonitor)

set(CMAKE_CXX_STANDARD 20)

add_executable(ResourceMonitor src/main.cpp
        src/monitoring/CpuTimes.h
        src/monitoring/CpuUsage.h
        src/monitoring/CpuUsage.cpp
        src/monitoring/windows/CpuTimesReader.h
        src/monitoring/windows/CpuTimesReader.cpp
        tests/CpuUsageTests.cpp
)
