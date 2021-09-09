/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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

#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/base/abstractfileinfo.h"
#include "dfm-base/dfm_base_global.h"

#include <QWidget>
#include <QSet>
#ifdef ENABLE_ASYNCINIT
#include <QFuture>
#endif

#include <functional>

class QUrl;
class QAbstractItemView;
DFMBASE_BEGIN_NAMESPACE
class SideBarView;
class SideBarModel;
class SideBarPrivate;
class SideBarItem;
class SideBar : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SideBar)
    SideBarPrivate * const d;

public:
    explicit SideBar(QWidget *parent = nullptr);
    virtual ~SideBar() override;
    QAbstractItemView *view(); // return m_sidebarView
    int addItem(SideBarItem *item);
    bool insertItem(const int index, SideBarItem *item);
    bool removeItem(SideBarItem *item);
    void setCurrentUrl(const QUrl &url);
    void changeEvent(QEvent *event) override;

Q_SIGNALS:
    void clickedItemUrl(const QUrl &url);
    void clickedItemIndex(const QModelIndex &index);
};

DFMBASE_END_NAMESPACE

#endif // SIDEBAR_H


