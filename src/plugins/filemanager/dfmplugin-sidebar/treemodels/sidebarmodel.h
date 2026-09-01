// SPDX-FileCopyrightText: 2020 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARMODEL_H
#define SIDEBARMODEL_H

#include "dfmplugin_sidebar_global.h"

#include <QStandardItemModel>
#include <QMutex>
#include <QPersistentModelIndex>
#include <QHash>

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarItem;
class SideBarItemSeparator;
class SidebarFileWatcher;
class SideBarModel : public QStandardItemModel
{
    Q_OBJECT
    friend class SidebarView;
    friend class SideBarView;

public:
    explicit SideBarModel(QObject *parent = nullptr);
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action,
                         int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent) override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    SideBarItem *itemFromIndex(const QModelIndex &index) const;
    SideBarItem *itemFromIndex(int index, const QModelIndex &parent = QModelIndex()) const;
    QList<SideBarItemSeparator *> groupItems() const;
    QList<SideBarItem *> subItems() const;
    QList<SideBarItem *> subItems(const QString &groupName) const;

    bool insertRow(int row, SideBarItem *item);
    int appendRow(SideBarItem *item, bool direct = true);
    bool removeRow(const QUrl &url);
    void updateRow(const QUrl &url, const ItemInfo &newInfo);
    QModelIndex findRowByUrl(const QUrl &url) const;
    QModelIndex findGroupIndex(const QString &name) const;
    QModelIndexList findRowsByUrlRecursive(const QUrl &url, const QModelIndex &parent) const;

    void addEmptyItem();

    // Partition sub-item management
    void onItemExpanded(const QModelIndex &index);
    void onItemCollapsed(const QModelIndex &index);
    void addSubItems(const QModelIndex &index, const QList<QUrl> &urls);

signals:
    void requestCollapseItem(const QModelIndex &index);

private slots:
    void onDirectoryCreated(const QUrl &parentUrl, const QUrl &url);
    void onDirectoryRemoved(const QUrl &parentUrl, const QUrl &url);
    void onDirectoryRenamed(const QUrl &parentUrl, const QUrl &oldUrl, const QUrl &newUrl);

    void addSubItem(const QModelIndex &index, const QUrl &url);
    void removeSubItem(const QModelIndex &index, const QUrl &url);
    // Set AT-SPI accessible name for sidebar items
    void setItemAccessibleName(SideBarItem *item) const;

private:
    QMutex locker;
    mutable SideBarItem *curDragItem { nullptr };

    SidebarFileWatcher *fileWatcher { nullptr };
    QHash<QUrl, QPersistentModelIndex> m_urlIndexMap;
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARMODEL_H
