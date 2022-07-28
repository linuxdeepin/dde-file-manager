/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhangsheng<zhangsheng@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#include "expandedItem.h"
#include "iconitemdelegate.h"
#include "utils/itemdelegatehelper.h"
#include "views/private/delegatecommon.h"

#include <QPainter>
#include <QDebug>

#include <cmath>

using namespace dfmplugin_workspace;

ExpandedItem::ExpandedItem(dfmplugin_workspace::IconItemDelegate *d, QWidget *parent)
    : QWidget(parent),
      delegate(d)
{
}

ExpandedItem::~ExpandedItem()
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

    pa.setOpacity(opacity);
    pa.setPen(option.palette.color(QPalette::BrightText));
    pa.setFont(option.font);

    if (!iconPixmap.isNull()) {
        pa.drawPixmap(iconGeometry().topLeft().toPoint(), iconPixmap);
    }

    if (option.text.isEmpty())
        return;

    const QMargins &margins = contentsMargins();

    QRect labelRect(kIconModeTextPadding + margins.left(),
                    margins.top() + iconHeight + kIconModeTextPadding + kIconModeIconSpacing,
                    width() - kIconModeTextPadding * 2 - margins.left() - margins.right(),
                    INT_MAX);

    QString str = delegate->displayFileName(index);

    const QList<QRectF> lines = delegate->drawText(index, &pa, str, labelRect, kIconModeRectRadius,
                                                   option.palette.brush(QPalette::Normal, QPalette::Highlight),
                                                   QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                   option.textElideMode, Qt::AlignCenter);

    textBounding = GlobalPrivate::boundingRect(lines).toRect();
}

QSize ExpandedItem::sizeHint() const
{
    return QSize(width(), static_cast<int>(std::floor(textGeometry().bottom() + contentsMargins().bottom())));
}

int ExpandedItem::heightForWidth(int width) const
{
    if (width != this->width())
        textBounding = QRect();

    return static_cast<int>(std::floor(textGeometry(width).bottom() + contentsMargins().bottom()));
}

qreal ExpandedItem::getOpacity() const
{
    return this->opacity;
}

void ExpandedItem::setOpacity(qreal opacity)
{
    if (qFuzzyCompare(opacity, this->opacity))
        return;

    this->opacity = opacity;
    update();
}

void ExpandedItem::setIconPixmap(const QPixmap &pixmap, int height)
{
    iconPixmap = pixmap;
    iconHeight = height;
    update();
}

QRectF ExpandedItem::getTextBounding() const
{
    return textBounding;
}

void ExpandedItem::setTextBounding(QRectF textBounding)
{
    this->textBounding = textBounding;
}

int ExpandedItem::getIconHeight() const
{
    return iconHeight;
}

void ExpandedItem::setIconHeight(int iconHeight)
{
    this->iconHeight = iconHeight;
}

bool ExpandedItem::getCanDeferredDelete() const
{
    return this->canDeferredDelete;
}

void ExpandedItem::setCanDeferredDelete(bool canDeferredDelete)
{
    this->canDeferredDelete = canDeferredDelete;
}

QModelIndex ExpandedItem::getIndex() const
{
    return this->index;
}

void ExpandedItem::setIndex(QModelIndex index)
{
    this->index = index;
}

QStyleOptionViewItem ExpandedItem::getOption() const
{
    return this->option;
}

void ExpandedItem::setOption(QStyleOptionViewItem opt)
{
    this->option = opt;
}

QRectF ExpandedItem::textGeometry(int width) const
{
    if (textBounding.isEmpty() && !option.text.isEmpty()) {
        const QMargins &margins = contentsMargins();

        if (width < 0)
            width = this->width();

        width -= (margins.left() + margins.right());

        QRect labelRect(kIconModeTextPadding + margins.left(),
                        iconHeight + kIconModeTextPadding + kIconModeRectRadius + margins.top(),
                        width - kIconModeTextPadding * 2,
                        INT_MAX);

        QString str = delegate->displayFileName(index);
        const QList<QRectF> &lines = delegate->drawText(index, nullptr, str, labelRect, kIconModeRectRadius, Qt::NoBrush,
                                                        QTextOption::WrapAtWordBoundaryOrAnywhere, option.textElideMode, Qt::AlignCenter);

        textBounding = GlobalPrivate::boundingRect(lines);
    }

    return textBounding;
}

QRectF ExpandedItem::iconGeometry() const
{
    const QRect &contentRect = contentsRect();

    if (!iconPixmap) {
        QRectF rect(contentRect);

        rect.setHeight(iconHeight);

        return rect;
    }

    QRectF iconRect(QPointF((contentRect.width() - iconPixmap.width() / iconPixmap.devicePixelRatio()) / 2.0,
                            (iconHeight - iconPixmap.height() / iconPixmap.devicePixelRatio()) / 2.0 + contentRect.top()),
                    iconPixmap.size() / iconPixmap.devicePixelRatio());

    return iconRect;
}
