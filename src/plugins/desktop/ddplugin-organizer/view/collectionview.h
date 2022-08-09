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
#include <QSharedPointer>

namespace ddplugin_organizer {

class FileProxyModel;
class CollectionItemDelegate;
class CollectionDataProvider;
class CanvasModelShell;
class CanvasViewShell;
class CanvasGridShell;
class CollectionViewPrivate;
class CollectionView : public QAbstractItemView
{
    Q_OBJECT
    friend class CollectionViewPrivate;
public:
    explicit CollectionView(const QString &uuid, CollectionDataProvider *dataProvider, QWidget *parent = nullptr);
    ~CollectionView() override;
    void setCanvasModelShell(CanvasModelShell *sh);
    void setCanvasViewShell(CanvasViewShell *sh);
    void setCanvasGridShell(CanvasGridShell *sh);
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
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void startDrag(Qt::DropActions supportedActions) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    QSharedPointer<CollectionViewPrivate> d = nullptr;
};

}

#endif // COLLECTIONVIEW_H
