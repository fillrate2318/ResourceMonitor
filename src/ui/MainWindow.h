#pragma once

#include <optional>

#include <QMainWindow>
#include <QStringList>

#include "../monitoring/CpuTimes.h"

class QLabel;
class QTimer;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void updateMetrics();

    void updateCpuUsage(QStringList& errors);
    void updateMemoryUsage(QStringList& errors);

    std::optional<CpuTimes> previousCpuTimes_ = std::nullopt;

    QLabel* cpuLabel_ = nullptr;
    QLabel* memoryLabel_ = nullptr;

    QTimer* timer_ = nullptr;
};
