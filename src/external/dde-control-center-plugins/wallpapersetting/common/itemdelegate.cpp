// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemdelegate.h"
#include "listview.h"

#include <DStyle>

#include <QPainter>

DWIDGET_USE_NAMESPACE
using namespace dfm_wallpapersetting;

ItemDelegate::ItemDelegate(ListView *parent) : QAbstractItemDelegate(parent)
{

}

ListView *ItemDelegate::view() const
{
    return qobject_cast<ListView *>(parent());
}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::Antialiasing);
    // draw selection state
    if (option.state & QStyle::State_Selected) {
        painter->save();
        const int penWidth = LISTVIEW_BORDER_WIDTH;
        QPen pen(option.palette.highlight(), penWidth);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        QRect focusRect = option.rect.marginsRemoved(QMargins(penWidth, penWidth, penWidth, penWidth));
        painter->drawRoundedRect(focusRect, LISTVIEW_BORDER_ROUND, LISTVIEW_BORDER_ROUND); //draw pen inclusive
        painter->restore();
    }

    const int margin = LISTVIEW_ICON_MARGIN;
    auto contentRect = option.rect.marginsRemoved(QMargins(margin, margin, margin, margin));

    // draw pixmap
    auto pix = index.data(Qt::DisplayRole).value<QPixmap>();
    if (!pix.isNull()) {
        painter->save();
        QPainterPath path;
        path.addRoundedRect(contentRect, LISTVIEW_ICON_ROUND, LISTVIEW_ICON_ROUND);
        painter->setClipPath(path);
        painter->drawPixmap(contentRect, pix);
        painter->restore();
    } else {
        // draw background
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(index.data(Qt::BackgroundRole).value<QColor>());
        painter->drawRoundedRect(contentRect, LISTVIEW_ICON_ROUND, LISTVIEW_ICON_ROUND);
        painter->restore();
    }
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(LISTVIEW_ICON_WIDTH + 2 * LISTVIEW_ICON_MARGIN,
                 LISTVIEW_ICON_HEIGHT + 2 * LISTVIEW_ICON_MARGIN);
}
