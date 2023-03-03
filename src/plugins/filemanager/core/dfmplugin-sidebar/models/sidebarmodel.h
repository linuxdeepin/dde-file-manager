// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARMODEL_H
#define SIDEBARMODEL_H

#include "dfmplugin_sidebar_global.h"

#include <QStandardItemModel>
#include <QMutex>

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarItem;
class SideBarModel : public QStandardItemModel
{
    Q_OBJECT
    friend class SidebarView;

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
    bool removeRow(const QUrl &url);
    void updateRow(const QUrl &url, const ItemInfo &newInfo);
    QStringList groups() const;
    int findRowByUrl(const QUrl &url);

private:
    QMutex locker;
};

DPSIDEBAR_END_NAMESPACE
#endif   // SIDEBARMODEL_H
