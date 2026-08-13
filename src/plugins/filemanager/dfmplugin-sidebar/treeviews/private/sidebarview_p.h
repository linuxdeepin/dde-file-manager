// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARVIEW_P_H
#define SIDEBARVIEW_P_H

#include "dfmplugin_sidebar_global.h"

#include <dfm-base/mimedata/dfmmimedata.h>

#include <QObject>
#include <QPoint>
#include <QModelIndex>
#include <QUrl>
#include <QDropEvent>
#include <QPalette>
#include <QStyle>
#include <QElapsedTimer>
#include <QPersistentModelIndex>
#include <QPointer>
#include <QHash>

class QVariantAnimation;

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarView;
class SideBarItem;
class SideBarViewPrivate : public QObject
{
    Q_OBJECT
    friend class SideBarView;
    SideBarView *const q;
    int previousRowCount { 0 };
    QPoint dropPos;
    QModelIndex previous;
    QModelIndex current;
    QPersistentModelIndex currentHoverIndex;
    bool isItemDragged = false;
    bool isRenderingDragPreview = false;
    QList<QUrl> urlsForDragEvent;
    QElapsedTimer lastOpTimer;
    QUrl draggedUrl;
    QString draggedGroup;
    QPoint dragPressPos;
    QPersistentModelIndex dragSourceIndex;
    QPersistentModelIndex previousPlaceholderParent;
    QPersistentModelIndex placeholderParent;
    int previousPlaceholderRow = -1;
    int placeholderRow = -1;
    QPointer<QVariantAnimation> placeholderAnimation;
    qreal placeholderAnimationProgress = 1.0;
    QVariantMap groupExpandState;
    QUrl sidebarUrl;
    DFMBASE_NAMESPACE::DFMMimeData dfmMimeData;
    QPalette originPalette;
    bool ignoreNextMouseRelease = false;

    // Track pending device-mount subscriptions so we can cancel stale ones.
    QHash<QUrl, int> pendingMountSubs;

    explicit SideBarViewPrivate(SideBarView *qq);
    bool checkOpTime();   // 检查当前操作与上次操作的时间间隔
    void notifyOrderChanged();
    void updateDFMMimeData(const QDropEvent *event);
    bool checkTargetEnable(const QUrl &targetUrl);
    bool canEnter(QDragEnterEvent *event);
    bool canMove(QDragMoveEvent *event);
    void updateHoverIndex(const QModelIndex &index);
    void clearHoverIndex();
    bool isCursorInsideIndex(const QModelIndex &index, const QPoint &fallbackPos) const;
    void setDragSourceIndex(const QModelIndex &index);
    void clearInternalDragState();
    int calculatePlaceholderRow(const QPoint &pos, const QMimeData *data) const;
    void updatePlaceholderRow(int row, const QModelIndex &parent);
    int dragItemOffset(const QModelIndex &index, int rowHeight) const;
    void expandPartitionItem(const QModelIndex &index, const QUrl &url);
    void cancelPendingMountSubscription(const QUrl &deviceUrl);

private Q_SLOTS:
    void currentChanged(const QModelIndex &curIndex);
    void onItemDoubleClicked(const QModelIndex &index);
    void expandItem(const QModelIndex &parentIndex, const QList<QUrl> &subFolders);
    void onExpandableChanged();

private:
    void setTransparentPalette();
    void restorePalette();
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARVIEW_P_H
