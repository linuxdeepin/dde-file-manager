// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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
    QModelIndex currentHoverIndex;
    bool isItemDragged = false;
    QList<QUrl> urlsForDragEvent;
    qint64 lastOpTime;   //上次操作的时间（ms）
    QUrl draggedUrl;
    QString draggedGroup;
    QVariantMap groupExpandState;
    QUrl sidebarUrl;
    DFMBASE_NAMESPACE::DFMMimeData dfmMimeData;

    explicit SideBarViewPrivate(SideBarView *qq);
    bool checkOpTime();   //检查当前操作与上次操作的时间间隔
    void notifyOrderChanged();
    void updateDFMMimeData(const QDropEvent *event);
    bool checkTargetEnable(const QUrl &targetUrl);
    bool canEnter(QDragEnterEvent *event);
    bool canMove(QDragMoveEvent *event);

private Q_SLOTS:
    void currentChanged(const QModelIndex &curIndex);
    void onItemDoubleClicked(const QModelIndex &index);
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARVIEW_P_H
