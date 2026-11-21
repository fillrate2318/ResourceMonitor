#include "ui/ProcessTableModel.h"

#include <QCoreApplication>
#include <QItemSelectionModel>

#include <iostream>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    ProcessTableModel model;

    model.setProcesses({{30, L"third", 1}, {10, L"kept", 2}, {2, L"gone", 3}});
    if (model.rowCount() != 3 || model.index(0, 1).data() != QVariant{2u}
        || model.index(1, 1).data() != QVariant{10u}
        || model.index(2, 1).data() != QVariant{30u}) {
        std::cerr << "Process rows were not ordered by numeric PID\n";
        return 1;
    }

    QItemSelectionModel selection(&model);
    selection.select(model.index(1, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    const QPersistentModelIndex kept = model.index(1, 0);
    const QPersistentModelIndex removed = model.index(0, 0);
    int dataChanges = 0;
    QObject::connect(&model, &QAbstractItemModel::dataChanged, &app, [&dataChanges] {
        ++dataChanges;
    });

    const std::vector<ProcessInfo> nextSnapshot = {
        {35, L"new tail", 4}, {10, L"updated", 8}, {5, L"new first", 1}, {6, L"new second", 1}
    };
    model.setProcesses(nextSnapshot);
    if (!kept.isValid() || kept.row() != 2 || removed.isValid()
        || !selection.isSelected(kept)) {
        std::cerr << "Process refresh did not preserve the selected process\n";
        return 1;
    }

    if (model.rowCount() != 4 || kept.data().toString() != QStringLiteral("updated")
        || model.index(2, 2).data() != QVariant{8u} || dataChanges != 1) {
        std::cerr << "Process refresh did not report the expected data change\n";
        return 1;
    }

    model.setProcesses(nextSnapshot);
    if (dataChanges != 1) {
        std::cerr << "An unchanged snapshot emitted dataChanged\n";
        return 1;
    }

    model.setProcesses({});
    if (model.rowCount() != 0 || kept.isValid() || selection.hasSelection()) {
        std::cerr << "An empty snapshot did not clear the model\n";
        return 1;
    }
}
