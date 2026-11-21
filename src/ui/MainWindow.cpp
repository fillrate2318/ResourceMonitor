#include "MainWindow.h"

#include <system_error>

#include <QLabel>
#include <QTimer>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QTableWidget>
#include <QAbstractItemView>
#include <QTabWidget>
#include <QTableView>

#include "../monitoring/MemoryUsage.h"
#include "../monitoring/windows/MemoryInfoReader.h"
#include "../monitoring/windows/CpuTopologyReader.h"
#include "../monitoring/windows/CpuFrequencyReader.h"
#include "../ui/ProcessTableModel.h"

namespace {
    constexpr double bytesPerGiB = 1073741824.0;
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    auto* centralWidget = new QWidget(this);
    auto* layout = new QVBoxLayout(centralWidget);

    cpuLabel_ = new QLabel(centralWidget);
    cpuLabel_->setText("Collecting data…");
    layout->addWidget(cpuLabel_);

    cpuTopologyLabel_ = new QLabel(centralWidget);
    layout->addWidget(cpuTopologyLabel_);

    cpuFrequencyLabel_ = new QLabel(centralWidget);
    layout->addWidget(cpuFrequencyLabel_);

    memoryLabel_ = new QLabel(centralWidget);
    layout->addWidget(memoryLabel_);

    logicalCpuTable_ = new QTableWidget(1, 2, centralWidget);
    logicalCpuTable_->setHorizontalHeaderLabels({"Logical processor", "Usage"});
    logicalCpuTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logicalCpuTable_->setItem(0, 0, new QTableWidgetItem("Collecting data…"));
    layout->addWidget(logicalCpuTable_, 1);

    auto* tabs = new QTabWidget(this);
    tabs->addTab(centralWidget, "Overview");

    auto* processView = new QTableView(this);
    tabs->addTab(processView, "Processes");

    processModel_ = new ProcessTableModel(processView);
    processView->setModel(processModel_);

    setCentralWidget(tabs);

    try {
        cpuUsageReader_ = std::make_unique<LogicalCpuUsageReader>();
        logicalCpuInitError_.clear();
    } catch (const std::system_error& e) {
        setCpuNotAvailable();
        logicalCpuInitError_ = formatError("Logical CPU: %1 (PDH 0x%2)", e);
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

    updateMemoryUsage(errors);
    updateLogicalCpuUsage(errors);
    updateCpuFrequency(errors);
    updateProcessList(errors);

    if (errors.isEmpty()) {
        statusBar()->clearMessage();
    } else {
        statusBar()->showMessage(errors.join(" | "));
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

void MainWindow::updateLogicalCpuUsage(QStringList& errors) {
    if (!cpuUsageReader_) {
        return;
    }

    try {
        std::optional<double> total = std::nullopt;

        const auto sample = cpuUsageReader_->sample(total);
        if (!sample) {
            return;
        }

        if (!total) {
            cpuLabel_->setText("CPU usage: N/A");
        } else {
            cpuLabel_->setText(QString("CPU usage: %1%").arg(*total, 0, 'f', 1));
        }

        logicalCpuTable_->setRowCount(static_cast<int>(sample->size()));

        int row = 0;
        for (const auto& cpu : *sample) {
            const auto name = QString("CPU %1").arg(cpu.number);
            const auto usage = cpu.usagePercent
                ? QString("%1%").arg(*cpu.usagePercent, 0, 'f', 1)
                : QString("N/A");

            logicalCpuTable_->setItem(row, 0, new QTableWidgetItem(name));
            logicalCpuTable_->setItem(row, 1, new QTableWidgetItem(usage));
            ++row;
        }
    } catch (const std::system_error& e) {
        setCpuNotAvailable();
        errors.append(formatError("Logical CPU: %1 (PDH 0x%2)", e));
    }
}

void MainWindow::updateCpuFrequency(QStringList &errors) {
    try {
        const auto freqs = readCpuFrequencies();
        if (freqs.empty()) {
            cpuFrequencyLabel_->setText(QString("CPU Frequency: N/A"));
            return;
        }
        const auto range = std::ranges::minmax_element(freqs.begin(), freqs.end());
        if (*range.min != *range.max) {
            cpuFrequencyLabel_->setText(QString("CPU Frequency: %1 GHz - %2 GHz")
                .arg(*range.min / 1000.0, 0, 'f', 2)
                .arg(*range.max / 1000.0, 0, 'f', 2));
        } else {
            cpuFrequencyLabel_->setText(QString("CPU Frequency: %1 GHz")
                .arg(*range.min / 1000.0, 0, 'f', 2));
        }
    } catch (const std::system_error& e) {
        cpuFrequencyLabel_->setText("CPU Frequency: N/A");
        errors.append(formatError("CPU Frequency: %1 (NTSTATUS 0x%2)", e));
    }
}

void MainWindow::updateProcessList(QStringList &errors) {
    try {
        processModel_->setProcesses(readProcesses());
    } catch (const std::system_error& e) {
        errors.append(formatError("Processes: %1 (Win32 0x%2)", e));
    }
}

//"Logical CPU: %1 (PDH 0x%2)"
QString MainWindow::formatError(const char* format, const std::system_error& e) const {
    const auto code = QString::number(static_cast<quint32>(e.code().value()), 16).toUpper();
    const auto message = QString(format)
        .arg(QString::fromLocal8Bit(e.what()))
        .arg(code);
    return message;
}

void MainWindow::setCpuNotAvailable() {
    logicalCpuTable_->clearContents();
    logicalCpuTable_->setRowCount(1);
    logicalCpuTable_->setItem(0, 0, new QTableWidgetItem("Logical CPU usage: N/A"));
    cpuLabel_->setText("CPU usage: N/A");
}
