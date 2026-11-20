cmake_minimum_required(VERSION 4.2)
project(ResourceMonitor)

set(CMAKE_CXX_STANDARD 20)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core)

qt_standard_project_setup()

add_executable(ResourceMonitor src/main.cpp
        src/monitoring/CpuTimes.h
        src/monitoring/CpuUsage.h
        src/monitoring/CpuUsage.cpp
        src/monitoring/windows/CpuTimesReader.h
        src/monitoring/windows/CpuTimesReader.cpp
        src/ui/MainWindow.cpp
        src/ui/MainWindow.h
        src/monitoring/MemoryInfo.h
        src/monitoring/windows/MemoryInfoReader.h
        src/monitoring/windows/MemoryInfoReader.cpp
        src/monitoring/MemoryUsage.h
        src/monitoring/MemoryUsage.cpp
        src/monitoring/CpuTopology.h
        src/monitoring/windows/CpuTopologyReader.h
        src/monitoring/windows/CpuTopologyReader.cpp
        src/monitoring/windows/LogicalCpuUsageReader.cpp
        src/monitoring/windows/LogicalCpuUsageReader.h
        src/monitoring/windows/CpuFrequencyReader.h
        src/monitoring/windows/CpuFrequencyReader.cpp
)

target_link_libraries(ResourceMonitor PRIVATE
        Qt6::Widgets Qt6::Core
        pdh
        PowrProf
)

enable_testing()

add_executable(MonitoringTests tests/MonitoringTests.cpp
        src/monitoring/CpuUsage.cpp
        src/monitoring/MemoryUsage.cpp
)

target_include_directories(MonitoringTests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

add_test(NAME MonitoringTests COMMAND MonitoringTests)
