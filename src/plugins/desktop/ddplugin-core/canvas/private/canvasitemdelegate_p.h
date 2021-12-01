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
#ifndef CANVASITEMDELEGATE_P_H
#define CANVASITEMDELEGATE_P_H

#include "canvasitemdelegate.h"

#include <QPointer>
#include <QTextDocument>

class ExpandedItem : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit ExpandedItem(CanvasItemDelegate *d, QWidget *parent = nullptr)
        : QWidget(parent), delegate(d)
    {
    }

    bool event(QEvent *ee) override;
    qreal opacity() const;
    void setOpacity(qreal opacity);
    void paintEvent(QPaintEvent *) override;
    QSize sizeHint() const override;
    int heightForWidth(int width) const override;
    void setIconPixmap(const QPixmap &pixmap, int height);
    QRectF iconGeometry() const;
    QRectF textGeometry(int width = -1) const;

public:
    QPixmap iconPixmap;
    int iconHeight = 0;
    mutable QRectF textBounding;
    QModelIndex index;
    QStyleOptionViewItem option;
    qreal m_opactity = 1;
    bool canDeferredDelete = true;
    CanvasItemDelegate *delegate;
};

class CanvasItemDelegatePrivate
{
    friend class CanvasItemDelegate;

public:
    explicit CanvasItemDelegatePrivate(CanvasItemDelegate *qq)
        : q(qq)
    {
        iconSizes << 32 << 48 << 64 << 96 << 128;
    }
    ~CanvasItemDelegatePrivate() {}

public:
    QPointer<ExpandedItem> expandedItem;
    mutable QModelIndex expandedIndex;
    mutable QModelIndex editingIndex;
    mutable QModelIndex lastAndExpandedIndex;

    QSize itemSizeHint;
    int textLineHeight = -1;
    int currentIconSizeIndex = -1;
    QList<int> iconSizes;

    QTextDocument *document = nullptr;
    CanvasItemDelegate *const q;
};

#endif   // CANVASITEMDELEGATE_P_H
