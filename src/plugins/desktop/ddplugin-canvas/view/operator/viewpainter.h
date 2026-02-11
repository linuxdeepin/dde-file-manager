// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWPAINTER_H
#define VIEWPAINTER_H

#include "view/canvasview_p.h"
#include "delegate/canvasitemdelegate.h"

#include <QPainter>

namespace ddplugin_canvas {

class ViewPainter : public QPainter
{
public:
    explicit ViewPainter(CanvasViewPrivate *dd);

public:
    inline CanvasView *view() const
    {
        return d->q;
    }

    inline CanvasProxyModel *model() const
    {
        return d->q->model();
    }

    inline CanvasSelectionModel *selectionModel() const
    {
        return d->q->selectionModel();
    }

    inline CanvasItemDelegate *itemDelegate() const
    {
        return d->q->itemDelegate();
    }

public:
    void paintFiles(QStyleOptionViewItem option, QPaintEvent *event);
    void drawGirdInfos();
    void drawFile(QStyleOptionViewItem option, const QModelIndex &index, const QPoint &gridPos);
    void drawDodge(QStyleOptionViewItem option);
    void drawMove(QStyleOptionViewItem option);
    void drawFileToPixmap(QPixmap *pix, QStyleOptionViewItem option, const QModelIndex &index);
    static QPixmap polymerize(QModelIndexList indexs, CanvasViewPrivate *d);

protected:
    static void drawDragText(QPainter *painter, const QString &str, const QRect &rect);
    static void drawEllipseBackground(QPainter *painter, const QRect &rect);

protected:
    CanvasViewPrivate *d = nullptr;
};

}

#endif   // VIEWPAINTER_H
