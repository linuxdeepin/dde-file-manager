// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
