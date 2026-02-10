// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASVIEW_H
#define CANVASVIEW_H

#include "ddplugin_canvas_global.h"

#include <QAbstractItemView>

namespace ddplugin_canvas {
class ShortcutOper;
class DragDropOper;
class ClickSelector;
class KeySelector;
class CanvasViewMenuProxy;
class CanvasProxyModel;
class CanvasSelectionModel;
class CanvasItemDelegate;
class CanvasViewPrivate;
class ViewHookInterface;
class CanvasView : public QAbstractItemView
{
    Q_OBJECT
    friend class ShortcutOper;
    friend class DragDropOper;
    friend class DodgeOper;
    friend class BoxSelector;
    friend class ClickSelector;
    friend class KeySelector;
    friend class ViewPainter;
    friend class CanvasViewMenuProxy;
    friend class CanvasViewBroker;
    friend class CanvasViewPrivate;

public:
    using CursorAction = QAbstractItemView::CursorAction;
    explicit CanvasView(QWidget *parent = nullptr);
    void initUI();
    void setViewHook(ViewHookInterface *);
    ViewHookInterface *viewHook() const;

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
    virtual void keyboardSearch(const QString &search) override;
    virtual void setSelectionModel(QItemSelectionModel *selectionModel) override;
    QList<QRect> itemPaintGeomertys(const QModelIndex &index) const;
    QRect expendedVisualRect(const QModelIndex &index) const;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
    WId winId() const;

    QModelIndex baseIndexAt(const QPoint &point) const;

public:
    void setScreenNum(const int screenNum);
    int screenNum() const;
    CanvasItemDelegate *itemDelegate() const;
    CanvasProxyModel *model() const;
    CanvasSelectionModel *selectionModel() const;
    void setGeometry(const QRect &rect);
    void updateGrid();
    void showGrid(bool v) const;

    void aboutToResortFiles();
    void filesResorted();

public Q_SLOTS:
    void reset() override;
    bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) override;
    void refresh(bool silent);
    void selectAll() override;
    void toggleSelect();

protected Q_SLOTS:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

protected:
    QRect itemRect(const QModelIndex &index) const;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *events) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void startDrag(Qt::DropActions supportedActions) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    CanvasViewPrivate *d;
};

}
#endif   // CANVASVIEW_H
