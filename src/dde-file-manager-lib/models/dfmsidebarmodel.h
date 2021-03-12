/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "dfmglobal.h"
#include <QStandardItemModel>

DFM_BEGIN_NAMESPACE

class DFMSideBarItem;
class DFMSideBarModel : public QStandardItemModel
{
public:
    explicit DFMSideBarModel(QObject *parent = nullptr);

    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    QModelIndex indexFromItem(const DFMSideBarItem * item) const;
    DFMSideBarItem *itemFromIndex(const QModelIndex &index) const;
    DFMSideBarItem *itemFromIndex(int index) const;

private:
    QByteArray generateMimeData(const QModelIndexList &indexes) const;
    int getRowIndexFromMimeData(const QByteArray &data) const;
};

DFM_END_NAMESPACE
