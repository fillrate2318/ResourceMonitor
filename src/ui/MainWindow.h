#pragma once

#include <optional>

#include <QMainWindow>

#include "../monitoring/CpuTimes.h"

class QLabel;
class QTimer;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void updateCpuUsage();

    std::optional<CpuTimes> previous_ = std::nullopt;
    QLabel* cpuLabel_;
    QTimer* timer_;

};
