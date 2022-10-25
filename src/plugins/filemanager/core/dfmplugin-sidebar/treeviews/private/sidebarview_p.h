/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
    bool fetchDragEventUrlsFromSharedMemory();
    bool checkOpTime();   //检查当前操作与上次操作的时间间隔
    void currentChanged(const QModelIndex &curIndex);
    void highlightAfterDraggedToSort();
    void notifyOrderChanged();
};

DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARVIEW_P_H
