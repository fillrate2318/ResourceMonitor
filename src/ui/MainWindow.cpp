#include "MainWindow.h"

#include <system_error>

#include <QLabel>
#include <QTimer>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

#include "../monitoring/windows/CpuTimesReader.h"
#include "../monitoring/windows/MemoryInfoReader.h"
#include "../monitoring/CpuUsage.h"

namespace {
    constexpr double bytesPerGiB = 1073741824.0;
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    auto* centralWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(centralWidget);

    cpuLabel_ = new QLabel(centralWidget);
    layout->addWidget(cpuLabel_);

    memoryLabel_ = new QLabel(centralWidget);
    layout->addWidget(memoryLabel_);

    layout->addStretch();

    setCentralWidget(centralWidget);

    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &MainWindow::updateMetrics);
    timer_->start(1000);

    updateMetrics();
}

void MainWindow::updateMetrics() {
    QStringList errors;

    updateCpuUsage(errors);
    updateMemoryUsage(errors);

    if (errors.isEmpty()) {
        statusBar()->clearMessage();
    } else {
        statusBar()->showMessage(errors.join(" | "));
    }
}

void MainWindow::updateCpuUsage(QStringList& errors) {
    try {
        const auto current = readCpuTimes();

        if (previousCpuTimes_) {
            if (const auto usage = calculateCpuUsage(*previousCpuTimes_, current)) {
                cpuLabel_->setText(QString("CPU usage: %1%").arg(*usage, 0, 'f', 1));
            } else {
                cpuLabel_->setText("CPU usage: N/A");
            }
        } else {
            cpuLabel_->setText("Collecting data…");
        }

        previousCpuTimes_ = current;
    } catch (const std::system_error& e) {
        cpuLabel_->setText("CPU usage: N/A");
        errors.append(QString("CPU sampling failed: %1").arg(
            QString::fromLocal8Bit(e.what())));
    }
}

void MainWindow::updateMemoryUsage(QStringList& errors) {
    try {
        const auto memoryInfo = readMemoryInfo();

        if (memoryInfo.totalBytes == 0 || memoryInfo.availableBytes > memoryInfo.totalBytes) {
            memoryLabel_->setText("Memory usage: N/A");
            errors.append(QString("Memory sampling returned invalid data"));
            return;
        }
        const std::uint64_t usedBytes = memoryInfo.totalBytes - memoryInfo.availableBytes;
        const double usagePercent = 100.0 * usedBytes / memoryInfo.totalBytes;

        memoryLabel_->setText(QString("RAM: %1 / %2 GiB (%3%)\nAvailable: %4 GiB")
            .arg(usedBytes / bytesPerGiB, 0, 'f', 1)
            .arg(memoryInfo.totalBytes / bytesPerGiB, 0, 'f', 1)
            .arg(usagePercent, 0, 'f', 1)
            .arg(memoryInfo.availableBytes / bytesPerGiB, 0, 'f', 1));
    } catch (const std::system_error& e) {
        memoryLabel_->setText("Memory usage: N/A");
        errors.append(QString("Memory sampling failed: %1").arg(
            QString::fromLocal8Bit(e.what())));
    }

}
