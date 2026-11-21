#pragma once

#include <memory>
#include <system_error>

#include <QMainWindow>

#include "../monitoring/windows/LogicalCpuUsageReader.h"
#include "../monitoring/windows/ProcessListReader.h"

class QLabel;
class QTableWidget;
class ProcessTableModel;
class QTimer;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void updateStaticMetrics();
    void updateDynamicMetrics();

    void updateMemoryUsage(QStringList& errors);
    void updateLogicalCpuUsage(QStringList& errors);
    void updateCpuFrequency(QStringList& errors);
    void updateProcessList(QStringList& errors);

    QString formatError(const char* format, const std::system_error& e) const;
    void setCpuNotAvailable();

    std::unique_ptr<LogicalCpuUsageReader> cpuUsageReader_;

    QLabel* cpuLabel_ = nullptr;
    QLabel* cpuTopologyLabel_ = nullptr;
    QLabel* cpuFrequencyLabel_ = nullptr;
    QLabel* memoryLabel_ = nullptr;

    ProcessTableModel* processModel_ = nullptr;

    QTableWidget* logicalCpuTable_ = nullptr;

    QTimer* timer_ = nullptr;

    QString topologyError_;
    QString logicalCpuInitError_;
};
