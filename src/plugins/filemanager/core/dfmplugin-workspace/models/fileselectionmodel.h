// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESELECTIONMODEL_H
#define FILESELECTIONMODEL_H

#include "dfmplugin_workspace_global.h"

#include <QItemSelectionModel>

namespace dfmplugin_workspace {

class FileSelectionModelPrivate;
class FileSelectionModel : public QItemSelectionModel
{
    Q_OBJECT

public:
    explicit FileSelectionModel(QAbstractItemModel *model = nullptr);
    explicit FileSelectionModel(QAbstractItemModel *model, QObject *parent);
    ~FileSelectionModel() override;

    bool isSelected(const QModelIndex &index) const;
    int selectedCount() const;
    QModelIndexList selectedIndexes() const;
    void clearSelectList();

public slots:
    void updateSelecteds();

public:
    void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command) override;
    void clear() override;

private:
    QScopedPointer<FileSelectionModelPrivate> d;
    Q_DECLARE_PRIVATE_D(d, FileSelectionModel)
};

}

#endif   // FILESELECTIONMODEL_H
