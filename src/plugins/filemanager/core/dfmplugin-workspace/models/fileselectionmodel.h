/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
