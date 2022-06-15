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
#ifndef COLLECTIONVIEW_H
#define COLLECTIONVIEW_H

#include "ddplugin_organizer_global.h"

#include <QAbstractItemView>

DDP_ORGANIZER_BEGIN_NAMESPACE

class FileProxyModel;
class CollectionItemDelegate;
class CollectionViewPrivate;
class CollectionView : public QAbstractItemView
{
    Q_OBJECT
    friend class CollectionViewPrivate;
public:
    explicit CollectionView(QWidget *parent = nullptr);
    QList<QUrl> urls() const;
    void setUrls(const QList<QUrl> &urls);
    QMargins cellMargins() const;
    FileProxyModel *model() const;
    CollectionItemDelegate *itemDelegate() const;

    QRect visualRect(const QModelIndex &index) const override;
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;
    QModelIndex indexAt(const QPoint &point) const override;

protected:
    QModelIndex moveCursor(CursorAction cursorAction,
                                   Qt::KeyboardModifiers modifiers) override;

    int horizontalOffset() const override;
    int verticalOffset() const override;

    bool isIndexHidden(const QModelIndex &index) const override;

    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override;
    QRegion visualRegionForSelection(const QItemSelection &selection) const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void updateGeometries() override;

private:
    void initUI();
    void updateRegionView();
    QList<QRect> itemPaintGeomertys(const QModelIndex &index) const;
    QRect itemRect(const QModelIndex &index) const;

private:
    CollectionViewPrivate *d;
};

DDP_ORGANIZER_END_NAMESPACE

#endif // COLLECTIONVIEW_H
