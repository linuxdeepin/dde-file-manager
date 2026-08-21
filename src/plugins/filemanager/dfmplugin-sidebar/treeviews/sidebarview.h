// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARVIEW_H
#define SIDEBARVIEW_H

#include "dfmplugin_sidebar_global.h"

#include <DTreeView>
#include <optional>

DWIDGET_USE_NAMESPACE

namespace dfmplugin_sidebar {

class SideBarItem;
class SideBarModel;
class SideBarViewPrivate;
class SideBarView : public DTreeView
{
    Q_OBJECT
    friend class SideBarViewPrivate;
    SideBarViewPrivate *const d = nullptr;

public:
    explicit SideBarView(QWidget *parent = nullptr);
    ~SideBarView() override;
    virtual SideBarModel *model() const;
    QModelIndex indexAt(const QPoint &p) const override;
    SideBarItem *itemAt(const QPoint &pt) const;
    QUrl urlAt(const QPoint &pt) const;
    void saveStateWhenClose();
    void setCurrentUrl(const QUrl &sidebarUrl);
    QUrl currentUrl() const;
    bool isCurrentUrlHighlightIndex(const QModelIndex &index) const;
    QModelIndex findItemIndex(const QUrl &url) const;
    QVariantMap groupExpandState() const;
    QModelIndex previousIndex() const;
    void setPreviousIndex(const QModelIndex &index);
    bool isDropTarget(const QModelIndex &index) const;
    bool isSideBarItemDragged() const;
    bool isDraggedSource(const QModelIndex &index) const;
    bool isRenderingDragPreview() const;
    int dragItemVerticalOffset(const QModelIndex &index, int rowHeight) const;
    bool isPartitionExpandable() const;

    // Sync the double-click rename trigger with the partition tree-view mode:
    // when the sidebar tree view (partitionExpandable) is enabled, double-click is
    // reserved for expanding/collapsing partitions and must not start inline renaming.
    void updateEditTriggers();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void startDrag(Qt::DropActions supportedActions) override;

    bool onDropData(QList<QUrl> srcUrls, QUrl dstUrl, Qt::DropAction action) const;
    Qt::DropAction canDropMimeData(SideBarItem *item, const QMimeData *data, Qt::DropActions actions) const;
    bool isAccepteDragEvent(QDropEvent *event) const;

private:
    inline QString dragEventUrls() const;

    mutable std::optional<bool> m_partitionExpandableCache;

public Q_SLOTS:
    void updateSeparatorVisibleState();
    void onChangeExpandState(const QModelIndex &index, bool expand);
    void onRequestCollapseItem(const QModelIndex &index);

Q_SIGNALS:
    void requestRemoveItem();
};

}

#endif   // SIDEBARVIEW_H
