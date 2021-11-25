/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
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

#include "dfm-base/dfm_base_global.h"

#include <QItemSelectionModel>

DFMBASE_BEGIN_NAMESPACE

class FileSelectionModelPrivate;
class FileSelectionModel : public QItemSelectionModel
{
    Q_OBJECT
    friend class FileSelectionModelPrivate;
    FileSelectionModelPrivate *const d;

public:
    explicit FileSelectionModel(QAbstractItemModel *model = nullptr);
    explicit FileSelectionModel(QAbstractItemModel *model, QObject *parent);

    bool isSelected(const QModelIndex &index) const;
    int selectedCount() const;
    QModelIndexList selectedIndexes() const;

protected:
    void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command) override;
    void clear() override;
};

DFMBASE_END_NAMESPACE

#endif   // FILESELECTIONMODEL_H
