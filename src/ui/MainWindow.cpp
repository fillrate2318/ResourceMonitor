#include "MainWindow.h"

#include <system_error>

#include <QLabel>
#include <QTimer>
#include <QStatusBar>

#include "../monitoring/windows/CpuTimesReader.h"
#include "../monitoring/CpuUsage.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Resource Monitor");
    cpuLabel_ = new QLabel(this);
    setCentralWidget(cpuLabel_);

    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &MainWindow::updateCpuUsage);
    timer_->start(1000);

    updateCpuUsage();
}

void MainWindow::updateCpuUsage() {
    try {
        const auto current = readCpuTimes();
        statusBar()->clearMessage();

        if (previous_) {
            if (const auto usage = calculateCpuUsage(*previous_, current)) {
                cpuLabel_->setText(QString("CPU usage: %1%").arg(*usage, 0, 'f', 1));
            } else {
                cpuLabel_->setText("CPU usage: N/A");
            }
        } else {
            cpuLabel_->setText("Collecting data…");
        }

        previous_ = current;
    } catch (const std::system_error& e) {
        cpuLabel_->setText("CPU usage: N/A");
        statusBar()->showMessage(QString("CPU sampling failed: %1").arg(
            QString::fromLocal8Bit(e.what())));
    }
}
