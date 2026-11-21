#include "ProcessTableModel.h"

#include <algorithm>
#include <utility>

int ProcessTableModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(processes_.size());
}

int ProcessTableModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : 3;
}

QVariant ProcessTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.model() != this || role != Qt::DisplayRole) {
        return {};
    }

    if (index.row() >= rowCount()) {
        return {};
    }

    const ProcessInfo& process = processes_[index.row()];

    switch (index.column()) {
        case 0:
            return QString::fromStdWString(process.name);
        case 1:
            return process.pid;
        case 2:
            return process.threadCount;
        default:
            return {};
    }
}

QVariant ProcessTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return {};
    }

    if (orientation != Qt::Horizontal) {
        return {};
    }

    switch (section) {
        case 0:
            return QString("Name");
        case 1:
            return QString("PID");
        case 2:
            return QString("Threads");
        default:
            return {};
    }
}

void ProcessTableModel::setProcesses(std::vector<ProcessInfo> processes) {
    std::ranges::sort(processes, {}, &ProcessInfo::pid);
    processes_.reserve(processes_.size() + processes.size());

    int row = 0;
    int incomingRow = 0;
    const int incomingCount = static_cast<int>(processes.size());
    while (row < rowCount() && incomingRow < incomingCount) {
        auto& current = processes_[row];
        auto& incoming = processes[incomingRow];

        if (current.pid < incoming.pid) {
            beginRemoveRows({}, row, row);
            processes_.erase(processes_.begin() + row);
            endRemoveRows();
            continue;
        }

        if (incoming.pid < current.pid) {
            beginInsertRows({}, row, row);
            processes_.insert(processes_.begin() + row, std::move(incoming));
            endInsertRows();
        } else if (current.name != incoming.name || current.threadCount != incoming.threadCount) {
            current = std::move(incoming);
            emit dataChanged(index(row, 0), index(row, 2), {Qt::DisplayRole});
        }

        ++row;
        ++incomingRow;
    }

    if (row < rowCount()) {
        beginRemoveRows({}, row, rowCount() - 1);
        processes_.erase(processes_.begin() + row, processes_.end());
        endRemoveRows();
    }

    if (incomingRow < incomingCount) {
        beginInsertRows({}, row, row + (incomingCount - incomingRow) - 1);
        while (incomingRow < incomingCount) {
            processes_.push_back(std::move(processes[incomingRow++]));
        }
        endInsertRows();
    }
}
