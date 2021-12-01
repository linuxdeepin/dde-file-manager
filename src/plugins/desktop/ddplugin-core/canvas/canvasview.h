/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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

#include "defaultdesktopfileinfo.h"
#include "canvasmodel.h"

#include <QAbstractItemView>

DSB_D_USE_NAMESPACE

class CanvasItemDelegate;
class CanvasViewPrivate;
class CanvasView : public QAbstractItemView
{
    Q_OBJECT
public:
    explicit CanvasView(QWidget *parent = nullptr);
    virtual QRect visualRect(const QModelIndex &index) const override;
    QRect visualRect(const QPoint &gridPos);
    virtual void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;
    virtual QModelIndex indexAt(const QPoint &point) const override;
    virtual QModelIndex moveCursor(CursorAction cursorAction,
                                   Qt::KeyboardModifiers modifiers) override;
    virtual int horizontalOffset() const override;
    virtual int verticalOffset() const override;
    virtual bool isIndexHidden(const QModelIndex &index) const override;
    virtual void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override;
    virtual QRegion visualRegionForSelection(const QItemSelection &selection) const override;

    virtual void paintEvent(QPaintEvent *event) override;

public:
    void setScreenNum(const int screenNum);
    void setScreenName(const QString name);
    int getScreenNum();
    QString getScreenName();

public:
    CanvasItemDelegate *itemDelegate() const;
    CanvasModel *canvasModel() const;
    void setGeometry(const QRect &rect);
    void updateCanvas();
    QString fileDisplayNameRole(const QModelIndex &index);

private:
    void initUI();
    void fileterAndRepaintLocalFiles(QPainter *painter, QStyleOptionViewItem &option, QPaintEvent *event);
    bool isRepaintFlash(QStyleOptionViewItem &option, QPaintEvent *event, const QPoint pos);

    void drawGirdInfos(QPainter *painter);
    void drawDodge(QPainter *painter);
    void drawLocalFile(QPainter *painter, QStyleOptionViewItem &option, bool enabled, const QPoint pos, const DFMDesktopFileInfoPointer &file);
    void drawSelectRect(QPainter *painter);
    void drawDragMove(QPainter *painter, QStyleOptionViewItem &option);

private:
    QScopedPointer<CanvasViewPrivate> d;
};
#endif   // CANVASVIEW_H
