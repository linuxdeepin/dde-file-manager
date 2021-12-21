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
#include "expandeditem.h"
#include "canvasitemdelegate.h"

#include <QEvent>
#include <QPainter>
#include <QTextLayout>

DSB_D_BEGIN_NAMESPACE

ExpandedItem::ExpandedItem(CanvasItemDelegate *d, QWidget *parent)
    : QWidget(parent)
    , delegate(d)
{

}

bool ExpandedItem::event(QEvent *ee)
{
    if (ee->type() == QEvent::DeferredDelete) {
        if (!canDeferredDelete) {
            ee->accept();

            return true;
        }
    }

    return QWidget::event(ee);
}

void ExpandedItem::paintEvent(QPaintEvent *)
{
    QPainter pa(this);

    pa.setOpacity(expandItemOpactity);
    pa.setPen(option.palette.color(QPalette::BrightText));
    pa.setFont(option.font);

    if (!iconPixmap.isNull()) {
        pa.drawPixmap(iconGeometry().topLeft().toPoint(), iconPixmap);
    }

    if (option.text.isEmpty())
        return;

    const QMargins &margins = contentsMargins();
    QRect labelRect(CanvasItemDelegate::kTextPadding + margins.left(), margins.top() + iconHeight +
                    CanvasItemDelegate::kTextPadding + CanvasItemDelegate::kIconSpacing,
                     width() - CanvasItemDelegate::kTextPadding * 2 - margins.left() - margins.right(), INT_MAX);
    QTextLayout tempLayout;
    tempLayout.setText(option.text);
    tempLayout.setFont(pa.font());
    auto backgroundBrush = option.palette.brush(QPalette::Normal, QPalette::Highlight);
    // todo(zy)
//    const QList<QRectF> &lines = delegate->drawText(index, &pa, &tempLayout, labelRect,
//                                                    CanvasItemDelegate::kTextPadding, backgroundBrush);

//    textBounding = delegate->boundingRect(lines).toRect();
}

QSize ExpandedItem::sizeHint() const
{
    int tempHeight = textGeometry().toRect().bottom() + contentsMargins().bottom();
    return QSize(width(), tempHeight);
}

int ExpandedItem::heightForWidth(int width) const
{
    if (width != this->width()) {
        textBounding = QRect();
    }

    return textGeometry(width).toRect().bottom() + contentsMargins().bottom();
}

void ExpandedItem::setIconPixmap(const QPixmap &pixmap, int height)
{
    iconPixmap = pixmap;
    iconHeight = height;
    update();
}

qreal ExpandedItem::opacity() const
{
    return expandItemOpactity;
}

void ExpandedItem::setOpacity(qreal opacity)
{
    if (qFuzzyCompare(opacity, expandItemOpactity))
        return;

    expandItemOpactity = opacity;
    update();
}

QRectF ExpandedItem::iconGeometry() const
{
    const QRect &content_rect = contentsRect();

    if (iconPixmap.isNull()) {
        QRectF rect(content_rect);

        rect.setHeight(iconHeight);

        return rect;
    }

    auto iconPixelRatioWidth = content_rect.width() - iconPixmap.width() / iconPixmap.devicePixelRatio();
    auto iconPixelRatioHeight = iconHeight - iconPixmap.height() / iconPixmap.devicePixelRatio();
    QRectF icon_rect(QPointF(iconPixelRatioWidth / 2.0,
                             iconPixelRatioHeight / 2.0 + content_rect.top()),
                     iconPixmap.size() / iconPixmap.devicePixelRatio());

    return icon_rect;
}

QRectF ExpandedItem::textGeometry(int width) const
{
    if (textBounding.isEmpty() && !option.text.isEmpty()) {
        const QMargins &margins = contentsMargins();

        if (width < 0)
            width = this->width();

        width -= (margins.left() + margins.right());
        QRect labelRect(CanvasItemDelegate::kTextPadding + margins.left(),
                        iconHeight + CanvasItemDelegate::kTextPadding
                        + CanvasItemDelegate::kIconSpacing + margins.top(),
                        width - CanvasItemDelegate::kTextPadding * 2, INT_MAX);

        QTextLayout tempLayout;
        tempLayout.setText(option.text);
        auto backgroundBrush = Qt::NoBrush;
        //todo(zy)
//        const QList<QRectF> &lines = delegate->drawText(index, nullptr, &tempLayout, labelRect,
//                                                        CanvasItemDelegate::kTextPadding, backgroundBrush);
//        textBounding = delegate->boundingRect(lines);
    }

    return textBounding;
}

DSB_D_END_NAMESPACE
