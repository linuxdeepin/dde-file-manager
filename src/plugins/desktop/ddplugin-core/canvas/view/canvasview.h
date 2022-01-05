/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
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
#ifndef CANVASVIEW_H
#define CANVASVIEW_H
#include "dfm_desktop_service_global.h"

#include <QAbstractItemView>

DSB_D_BEGIN_NAMESPACE
class ClickSelecter;
class KeySelecter;
class CanvasModel;
class CanvasSelectionModel;
class CanvasItemDelegate;
class CanvasViewPrivate;
class CanvasView : public QAbstractItemView
{
    Q_OBJECT
    friend class BoxSelecter;
    friend class ClickSelecter;
    friend class KeySelecter;
    friend class ViewPainter;
    friend class CanvasViewPrivate;
public:
    using CursorAction = QAbstractItemView::CursorAction;
    explicit CanvasView(QWidget *parent = nullptr);
    void initUI();
public:
    virtual QRect visualRect(const QModelIndex &index) const override;
    virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;
    virtual QModelIndex indexAt(const QPoint &point) const override;
    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
    virtual int horizontalOffset() const override;
    virtual int verticalOffset() const override;
    virtual bool isIndexHidden(const QModelIndex &index) const override;
    QT_DEPRECATED virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override;
    virtual QRegion visualRegionForSelection(const QItemSelection &selection) const override;
    QList<QRect> itemPaintGeomertys(const QModelIndex &index) const;
public:
    void setScreenNum(const int screenNum);
    int screenNum() const;
    CanvasItemDelegate *itemDelegate() const;
    CanvasModel *model() const;
    CanvasSelectionModel *selectionModel() const;
    void setGeometry(const QRect &rect);
    void updateGrid();
public:
    bool isTransparent(const QModelIndex &index) const;
    QList<QIcon> additionalIcon(const QModelIndex &index) const;
protected:
    QRect itemRect(const QModelIndex &index) const;
protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *events) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QScopedPointer<CanvasViewPrivate> d;
};

DSB_D_END_NAMESPACE
#endif   // CANVASVIEW_H
