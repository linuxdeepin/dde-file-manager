/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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

#include "dfm_base_global.h"
#include <QObject>
#include <QPoint>
#include <QModelIndex>

DFMBASE_BEGIN_NAMESPACE
class SideBarView;
class SideBarViewPrivate : public QObject
{
    Q_OBJECT
    friend class SideBarView;
    SideBarView *const q;
    int previousRowCount;
    QPoint dropPos;
    QString dragItemName;
    int dragRow;
    QModelIndex previous;
    QModelIndex current;
    QString strItemUniqueKey;
    QList<QUrl> urlsForDragEvent;
    qint64 lastOpTime; //上次操作的时间（ms）

    explicit SideBarViewPrivate(SideBarView *qq);
    bool fetchDragEventUrlsFromSharedMemory();
    bool checkOpTime(); //检查当前操作与上次操作的时间间隔
    void currentChanged(const QModelIndex &previous);
};

DFMBASE_END_NAMESPACE

#endif // SIDEBARVIEW_P_H
