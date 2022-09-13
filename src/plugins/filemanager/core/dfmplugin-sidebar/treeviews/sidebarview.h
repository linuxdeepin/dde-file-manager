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
#ifndef SIDEBARVIEW_H
#define SIDEBARVIEW_H

#include "dfmplugin_sidebar_global.h"

#include <DTreeView>

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
    virtual SideBarModel *model() const;
    QModelIndex indexAt(const QPoint &p) const override;
    QModelIndex getPreviousIndex() const;
    QModelIndex getCurrentIndex() const;
    SideBarItem *itemAt(const QPoint &pt) const;
    QUrl urlAt(const QPoint &pt) const;
    void saveStateWhenClose();
    void setCurrentUrl(const QUrl &sidebarUrl);
    QUrl currentUrl() const;
    QModelIndex findItemIndex(const QUrl &url) const;
    QVariantMap groupExpandState() const;
    QMap<QUrl, QPair<QString, int>> defaultItemNames();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    bool onDropData(QList<QUrl> srcUrls, QUrl dstUrl, Qt::DropAction action) const;
    Qt::DropAction canDropMimeData(SideBarItem *item, const QMimeData *data, Qt::DropActions actions) const;
    bool isAccepteDragEvent(QDropEvent *event);

private:
    inline QString dragEventUrls() const;

public Q_SLOTS:
    void updateSeparatorVisibleState();
    void onChangeExpandState(const QModelIndex &index, bool expand);

Q_SIGNALS:
    void requestRemoveItem();

private:
    QUrl sidebarUrl;
};

}

#endif   // SIDEBARVIEW_H
