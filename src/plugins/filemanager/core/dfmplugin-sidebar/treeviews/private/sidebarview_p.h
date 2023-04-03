// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARVIEW_P_H
#define SIDEBARVIEW_P_H

#include "dfmplugin_sidebar_global.h"

#include <QObject>
#include <QPoint>
#include <QModelIndex>
#include <QUrl>

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
    QList<QUrl> urlsForDragEvent;
    qint64 lastOpTime;   //上次操作的时间（ms）
    QUrl draggedUrl;
    QString draggedGroup;
    QVariantMap groupExpandState;
    QUrl sidebarUrl;

    explicit SideBarViewPrivate(SideBarView *qq);
    bool checkOpTime();   //检查当前操作与上次操作的时间间隔
    void currentChanged(const QModelIndex &curIndex);
    void notifyOrderChanged();
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARVIEW_P_H
