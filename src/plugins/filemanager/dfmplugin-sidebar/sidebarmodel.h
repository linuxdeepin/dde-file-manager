/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#pragma once

#include <QStandardItemModel>

class SideBarItem;
class SideBarModelPrivate;
class SideBarModel : public QStandardItemModel
{
    Q_OBJECT
    friend class SideBarModelPrivate;
    SideBarModelPrivate *const d;

public:
    explicit SideBarModel(QObject *parent = nullptr);
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action,
                         int row, int column, const QModelIndex &parent) const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    SideBarItem *itemFromIndex(const QModelIndex &index) const;
    SideBarItem *itemFromIndex(int index) const;
    bool insertRow(int row, SideBarItem *item);
    int appendRow(SideBarItem *item);
    bool removeRow(SideBarItem *item);
    QStringList groups() const;
};
