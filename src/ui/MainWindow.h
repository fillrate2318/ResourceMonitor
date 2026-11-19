#pragma once

#include <memory>
#include <system_error>

#include <QMainWindow>

#include "../monitoring/windows/LogicalCpuUsageReader.h"

class QLabel;
class QTableWidget;
class QTimer;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void updateStaticMetrics();
    void updateDynamicMetrics();

    void updateMemoryUsage(QStringList& errors);
    void updateLogicalCpuUsage(QStringList& errors);

    QString formatLogicalCpuError(const std::system_error& e) const;
    void setUndefinedLogicalCpuUsage();

    std::unique_ptr<LogicalCpuUsageReader> cpuUsageReader_;

    QLabel* cpuLabel_ = nullptr;
    QLabel* cpuTopologyLabel_ = nullptr;
    QLabel* memoryLabel_ = nullptr;

    QTableWidget* logicalCpuTable_ = nullptr;

    QTimer* timer_ = nullptr;

    QString topologyError_;
    QString logicalCpuInitError_;
};
