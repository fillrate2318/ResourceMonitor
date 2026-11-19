#include "MainWindow.h"

#include <system_error>

#include <QLabel>
#include <QTimer>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QTableWidget>
#include <QAbstractItemView>

#include "../monitoring/CpuUsage.h"
#include "../monitoring/MemoryUsage.h"
#include "../monitoring/windows/CpuTimesReader.h"
#include "../monitoring/windows/MemoryInfoReader.h"
#include "../monitoring/windows/CpuTopologyReader.h"

namespace {
    constexpr double bytesPerGiB = 1073741824.0;
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    auto* centralWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(centralWidget);

    cpuLabel_ = new QLabel(centralWidget);
    layout->addWidget(cpuLabel_);

    cpuTopologyLabel_ = new QLabel(centralWidget);
    layout->addWidget(cpuTopologyLabel_);

    memoryLabel_ = new QLabel(centralWidget);
    layout->addWidget(memoryLabel_);

    logicalCpuTable_ = new QTableWidget(1, 2, centralWidget);
    logicalCpuTable_->setHorizontalHeaderLabels({"Logical processor", "Usage"});
    logicalCpuTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logicalCpuTable_->setItem(0, 0, new QTableWidgetItem("Collecting data…"));

    layout->addWidget(logicalCpuTable_, 1);

    setCentralWidget(centralWidget);

    try {
        cpuUsageReader_ = std::make_unique<LogicalCpuUsageReader>();
        logicalCpuInitError_.clear();
    }
    catch (const std::system_error& e) {
        setUndefinedLogicalCpuUsage();
        logicalCpuInitError_ = formatLogicalCpuError(e);
    }

    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &MainWindow::updateDynamicMetrics);
    timer_->start(1000);

    updateStaticMetrics();
    updateDynamicMetrics();
}

void MainWindow::updateStaticMetrics() {
    try {
        const auto cpuTopology = readCpuTopology();
        topologyError_.clear();
        cpuTopologyLabel_->setText(QString("Physical core count: %1\nLogical processor count: %2")
            .arg(cpuTopology.physicalCoreCount).arg(cpuTopology.logicalProcessorCount));
    } catch (const std::runtime_error& e) {
        cpuTopologyLabel_->setText("CPU topology: N/A");
        topologyError_ = QString("CPU topology failed: %1")
            .arg(QString::fromLocal8Bit(e.what()));
    }
}

void MainWindow::updateDynamicMetrics() {
    QStringList errors;

    if (!topologyError_.isEmpty()) {
        errors.append(topologyError_);
    }

    if (!logicalCpuInitError_.isEmpty()) {
        errors.append(logicalCpuInitError_);
    }

    updateCpuUsage(errors);
    updateMemoryUsage(errors);
    updateLogicalCpuUsage(errors);

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
        const auto memoryUsage = calculateMemoryUsage(memoryInfo);

        if (!memoryUsage) {
            memoryLabel_->setText("Memory usage: N/A");
            errors.append(QString("Memory sampling returned invalid data"));
            return;
        }

        memoryLabel_->setText(QString("RAM: %1 / %2 GiB (%3%)\nAvailable: %4 GiB")
            .arg(memoryUsage->usedBytes / bytesPerGiB, 0, 'f', 1)
            .arg(memoryInfo.totalBytes / bytesPerGiB, 0, 'f', 1)
            .arg(memoryUsage->usagePercent, 0, 'f', 1)
            .arg(memoryInfo.availableBytes / bytesPerGiB, 0, 'f', 1));
    } catch (const std::system_error& e) {
        memoryLabel_->setText("Memory usage: N/A");
        errors.append(QString("Memory sampling failed: %1").arg(
            QString::fromLocal8Bit(e.what())));
    }

}

void MainWindow::updateLogicalCpuUsage(QStringList &errors) {
    if (!cpuUsageReader_) {
        return;
    }

    try {
        const auto sample = cpuUsageReader_->sample();
        if (!sample) {
            return;
        }

        logicalCpuTable_->setRowCount(sample->size());

        int row = 0;
        for (const auto& cpu : *sample) {
            const auto name = QString::fromStdWString(cpu.processorId);
            const auto usage = cpu.usagePercent
                ? QString("%1%").arg(*cpu.usagePercent, 0, 'f', 1)
                : QString("N/A");

            logicalCpuTable_->setItem(row, 0, new QTableWidgetItem(name));
            logicalCpuTable_->setItem(row, 1, new QTableWidgetItem(usage));
            ++row;
        }
    }
    catch (const std::system_error& e) {
        setUndefinedLogicalCpuUsage();
        errors.append(formatLogicalCpuError(e));
    }
}

QString MainWindow::formatLogicalCpuError(const std::runtime_error &e) const {
    const auto code = QString::number(static_cast<quint32>(e.code().value()), 16).toUpper();
    const auto message = QString("Logical CPU: %1 (PDH 0x%2)")
        .arg(QString::fromLocal8Bit(e.what()))
        .arg(code);
    return message;
}

void MainWindow::setUndefinedLogicalCpuUsage() {
    logicalCpuTable_->clearContents();
    logicalCpuTable_->setRowCount(1);
    logicalCpuTable_->setItem(0, 0, new QTableWidgetItem("Logical CPU usage: N/A"));
}
