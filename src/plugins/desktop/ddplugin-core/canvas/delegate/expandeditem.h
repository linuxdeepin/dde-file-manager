/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef EXPANDEDITEM_H
#define EXPANDEDITEM_H

#include "dfm_desktop_service_global.h"

#include <QWidget>
#include <QModelIndex>
#include <QStyleOptionViewItem>

DSB_D_BEGIN_NAMESPACE

class CanvasItemDelegate;
class ExpandedItem : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    explicit ExpandedItem(CanvasItemDelegate *d, QWidget *parent = nullptr);
    bool event(QEvent *ee) override;
    void paintEvent(QPaintEvent *) override;
    QSize sizeHint() const override;
    int heightForWidth(int width) const override;
    void setIconPixmap(const QPixmap &pixmap, int height);
    qreal opacity() const;
    void setOpacity(qreal opacity);
    QRectF iconGeometry() const;
    QRectF textGeometry(int width = -1) const;

public:
    int iconHeight { 0 };
    qreal expandItemOpactity { 1 };
    bool canDeferredDelete { true };
    QPixmap iconPixmap;
    mutable QRectF textBounding;
    QModelIndex index;
    QStyleOptionViewItem option;
    CanvasItemDelegate *delegate { nullptr };
};

DSB_D_END_NAMESPACE
#endif // EXPANDEDITEM_H
