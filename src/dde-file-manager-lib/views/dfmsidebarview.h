// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
