// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESELECTIONMODEL_P_H
#define FILESELECTIONMODEL_P_H

#include "models/fileselectionmodel.h"

#include <QTimer>

namespace dfmplugin_workspace {

class FileSelectionModelPrivate : public QObject
{
    Q_OBJECT
    friend class FileSelectionModel;
    FileSelectionModel *const q;

public:
    explicit FileSelectionModelPrivate(FileSelectionModel *qq);
    ~FileSelectionModelPrivate() override;

    mutable QModelIndexList selectedList;
    QItemSelection selection;
    QModelIndex firstSelectedIndex;
    QModelIndex lastSelectedIndex;
    QItemSelectionModel::SelectionFlags currentCommand;
    QTimer timer;
};

}

#endif   // FILESELECTIONMODEL_P_H
