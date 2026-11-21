#include "ProcessTableModel.h"

int ProcessTableModel::rowCount(const QModelIndex& parent) const {
    return !parent.isValid() ? processes_.size() : 0;
}

int ProcessTableModel::columnCount(const QModelIndex& parent) const {
    return !parent.isValid() ? 3 : 0;
}

QVariant ProcessTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return {};
    }

    if (role != Qt::DisplayRole) {
        return {};
    }

    if (index.row() < 0 ||
        static_cast<size_t>(index.row()) >= processes_.size()) {
        return {};
    }

    const ProcessInfo& process = processes_[index.row()];

    switch (index.column())
    {
        case 0:
            return QString::fromStdWString(process.name);
        case 1:
            return QString::number(process.pid);
        case 2:
            return QString::number(process.threadCount);
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
    beginResetModel();
    processes_ = std::move(processes);
    endResetModel();
}
