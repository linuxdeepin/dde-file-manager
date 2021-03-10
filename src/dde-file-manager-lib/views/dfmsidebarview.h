/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
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

#pragma once

#include "dfmglobal.h"
#include <DListView>

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE
class DFMSideBarItem;
typedef QDropEvent DFMDragEvent;
class DFMSideBarView : public DListView
{
    Q_OBJECT
public:
    explicit DFMSideBarView(QWidget *parent = nullptr);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    QModelIndex indexAt(const QPoint &p) const override;
    QModelIndex getPreviousIndex() const;
    QModelIndex getCurrentIndex() const;
    void currentChanged(const QModelIndex &previous);
    DFMSideBarItem *itemAt(const QPoint &pt);
    void updateItemUniqueKey(const QModelIndex &index);

protected:
    bool onDropData(DUrlList srcUrls, DUrl dstUrl, Qt::DropAction action) const;
    Qt::DropAction canDropMimeData(DFMSideBarItem *item, const QMimeData *data, Qt::DropActions actions) const;
    bool isAccepteDragEvent(DFMDragEvent *event);
signals:
    void requestRemoveItem();

private slots:
    void onRowCountChanged();

private:
    bool fetchDragEventUrlsFromSharedMemory();
    //检查当前操作与上次操作的时间间隔
    bool checkOpTime();
    int previousRowCount;
    QPoint dropPos;
    QString dragItemName;
    int dragRow;
    QModelIndex m_previous;
    QModelIndex m_current;
    QString m_strItemUniqueKey;

    QList<QUrl> m_urlsForDragEvent;

    //上次操作的时间（ms）
    qint64 m_lastOpTime;
};

DFM_END_NAMESPACE
