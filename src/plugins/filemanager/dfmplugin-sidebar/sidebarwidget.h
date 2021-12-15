/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include "dfm-base/interfaces/abstractframe.h"

#include <QWidget>

class QAbstractItemView;
class SideBarView;
class SideBarModel;
class SideBarItem;
class SideBarWidget : public dfmbase::AbstractFrame
{
    Q_OBJECT
public:
    explicit SideBarWidget(QFrame *parent = nullptr);
    void setCurrentUrl(const QUrl &url) override;
    void changeEvent(QEvent *event) override;

    QAbstractItemView *view();
    int addItem(SideBarItem *item);
    bool insertItem(const int index, SideBarItem *item);
    bool removeItem(SideBarItem *item);

Q_SIGNALS:
    void clickedItemUrl(const QUrl &url);
    void clickedItemIndex(const QModelIndex &index);
    void customContextMenu(const QUrl &url, const QPoint &pos);

private Q_SLOTS:
    void onItemClicked(const QModelIndex &index);
    void customContextMenuCall(const QPoint &pos);

private:
    void initializeUi();
    void initConnect();

private:
    SideBarView *sidebarView { nullptr };
    SideBarModel *sidebarModel { nullptr };
};

#endif   // SIDEBARWIDGET_H
