#pragma once

#include <qabstractitemmodel.h>
#include <vector>

#include "../monitoring/ProcessInfo.h"

class ProcessTableModel : public QAbstractTableModel {
public:
    using QAbstractTableModel::QAbstractTableModel;

    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void setProcesses(std::vector<ProcessInfo> processes);

private:
    std::vector<ProcessInfo> processes_;
};
