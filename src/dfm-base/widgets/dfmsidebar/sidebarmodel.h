/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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

#include "dfm-base/dfm_base_global.h"

#include <QStandardItemModel>
DFMBASE_BEGIN_NAMESPACE
class SideBarItem;
class SideBarModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit SideBarModel(QObject *parent = nullptr);

    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    QModelIndex indexFromItem(const SideBarItem * item) const;
    SideBarItem *itemFromIndex(const QModelIndex &index) const;
    SideBarItem *itemFromIndex(int index) const;

private:
    QByteArray generateMimeData(const QModelIndexList &indexes) const;
    int getRowIndexFromMimeData(const QByteArray &data) const;
};
DFMBASE_END_NAMESPACE
