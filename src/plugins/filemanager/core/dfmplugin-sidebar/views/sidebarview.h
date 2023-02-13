// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARVIEW_H
#define SIDEBARVIEW_H

#include "dfmplugin_sidebar_global.h"

#include <DListView>

DWIDGET_USE_NAMESPACE

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarItem;
class SideBarModel;
class SideBarViewPrivate;
class SideBarView : public DListView
{
    Q_OBJECT
    friend class SideBarViewPrivate;
    SideBarViewPrivate *const d;

public:
    explicit SideBarView(QWidget *parent = nullptr);
    virtual SideBarModel *model() const;
    QModelIndex indexAt(const QPoint &p) const override;
    QModelIndex getPreviousIndex() const;
    QModelIndex getCurrentIndex() const;
    SideBarItem *itemAt(const QPoint &pt) const;
    QUrl urlAt(const QPoint &pt) const;

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

Q_SIGNALS:
    void requestRemoveItem();
};
DPSIDEBAR_END_NAMESPACE

#endif   // SIDEBARVIEW_H
