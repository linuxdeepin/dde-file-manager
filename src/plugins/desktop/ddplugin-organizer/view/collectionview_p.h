/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef COLLECTIONVIEW_P_H
#define COLLECTIONVIEW_P_H

#include "view/collectionview.h"
#include "mode/collectiondataprovider.h"

#include <QAtomicInteger>
#include <QTimer>
#include <QUrl>
#include <QPointer>

namespace ddplugin_organizer {

class CollectionViewPrivate : public QObject
{
    Q_OBJECT
public:
    explicit CollectionViewPrivate(const QString &uuid, CollectionDataProvider *dataProvider, CollectionView *qq, QObject *parent = nullptr);
    ~CollectionViewPrivate();

    void initUI();
    void initConnect();
    void updateRegionView();
    void updateViewSizeData(const QSize &viewSize, const QMargins &viewMargins, const QSize &itemSize);
    void updateVerticalBarRange();
    QList<QRect> itemPaintGeomertys(const QModelIndex &index) const;
    QRect itemRect(const QModelIndex &index) const;
    QRect visualRect(const QPoint &pos) const;

    int verticalScrollToValue(const QModelIndex &index, const QRect &rect, QAbstractItemView::ScrollHint hint) const;
    QItemSelection selection(const QRect &rect) const;
    void selectItems(const QList<QUrl> &fileUrl) const;

    QPoint pointToPos(const QPoint &point) const;
    QPoint posToPoint(const QPoint &pos) const;
    int posToNode(const QPoint &pos) const;
    QPoint nodeToPos(const int node) const;

    void checkTouchDarg(QMouseEvent *event);
    bool isDelayDrag() const;
    QPixmap polymerizePixmap(QModelIndexList indexs) const;

    bool checkClientMimeData(QDragEnterEvent *event) const;
    bool checkXdndDirectSave(QDragEnterEvent *event) const;
    void preproccessDropEvent(QDropEvent *event, const QUrl &targetUrl) const;
    void handleMoveMimeData(QDropEvent *event, const QUrl &url);
    bool drop(QDropEvent *event);

private:
    void updateRowCount(const int &viewHeight, const int &itemHeight);
    void updateColumnCount(const int &viewWidth, const int &itemWidth);
    void updateCellMargins(const QSize &itemSize, const QSize &cellSize);
    void updateViewMargins(const QSize &viewSize, const QMargins &oldMargins);

    void drawDragText(QPainter *painter, const QString &str, const QRect &rect) const;
    void drawEllipseBackground(QPainter *painter, const QRect &rect) const;
    void updateTarget(const QMimeData *data, const QUrl &url);

    bool dropFilter(QDropEvent *event);
    bool dropClientDownload(QDropEvent *event) const;
    bool dropDirectSaveMode(QDropEvent *event) const;
    bool dropBetweenCollection(QDropEvent *event) const;
    bool dropFromCanvas(QDropEvent *event) const;
    bool dropMimeData(QDropEvent *event) const;
    bool dropFiles(QDropEvent *event) const;

private slots:
    void onItemsChanged(const QString &key);

public:
    CollectionView *q = nullptr;
    CanvasModelShell *canvasModelShell = nullptr;
    CanvasViewShell *canvasViewShell = nullptr;
    CanvasGridShell *canvasGridShell = nullptr;

    QString id;
    QPointer<CollectionDataProvider> provider = nullptr;

    int space = 0;
    QMargins viewMargins;
    QMargins cellMargins = QMargins(2, 2, 2, 2);
    int rowCount = 1;
    int columnCount = 1;
    int cellWidth = 1;
    int cellHeight = 1;

    QAtomicInteger<bool> canUpdateVerticalBarRange = true;
    QAtomicInteger<bool> needUpdateVerticalBarRange = false;
    bool showGrid = false;

    QTimer touchDragTimer;
    QUrl dropTargetUrl;
};

}

#endif // COLLECTIONVIEW_P_H
