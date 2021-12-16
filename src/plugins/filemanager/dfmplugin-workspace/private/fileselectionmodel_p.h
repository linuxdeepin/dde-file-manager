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
#ifndef FILESELECTIONMODEL_P_H
#define FILESELECTIONMODEL_P_H

#include "fileselectionmodel.h"

#include <QTimer>

class FileSelectionModelPrivate : public QObject
{
    Q_OBJECT
    friend class FileSelectionModel;
    FileSelectionModel *const q;

public:
    explicit FileSelectionModelPrivate(FileSelectionModel *qq);

    void updateSelecteds();

    mutable QModelIndexList selectedList;
    QItemSelection selection;
    QModelIndex firstSelectedIndex;
    QModelIndex lastSelectedIndex;
    QItemSelectionModel::SelectionFlags currentCommand;
    QTimer timer;
};

#endif   // FILESELECTIONMODEL_P_H
