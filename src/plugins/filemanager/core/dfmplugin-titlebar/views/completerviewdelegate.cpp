// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "completerviewdelegate.h"
#include "models/completerviewmodel.h"
#include "completerview.h"

#include <QPainter>
#include <QCompleter>

using namespace dfmplugin_titlebar;
DWIDGET_USE_NAMESPACE

CompleterViewDelegate::CompleterViewDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
}

void CompleterViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid())
        return DStyledItemDelegate::paint(painter, option, index);
    QStyleOptionViewItem opt = option;
    DStyledItemDelegate::initStyleOption(&opt, index);
    painter->setRenderHint(QPainter::Antialiasing);

    // prepare
    QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
            ? QPalette::Normal
            : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

    // draw background
    if (option.showDecorationSelected && (option.state & (QStyle::State_Selected | QStyle::State_MouseOver))) {
        painter->fillRect(option.rect, option.palette.brush(cg, QPalette::Highlight));
    }

    // draw icon
    paintItemIcon(painter, option, index);

    // draw text
    if (option.state & (QStyle::State_Selected | QStyle::State_MouseOver)) {
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    painter->setFont(option.font);
    auto text = index.data(Qt::DisplayRole).toString();
    if (text.contains('\n'))
        text = text.replace('\n', ' ');
    painter->drawText(option.rect.adjusted(kTextLeftPadding, 0, 0, 0), Qt::AlignVCenter, text);
}

QSize CompleterViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(kItemHeight);

    return s;
}

void CompleterViewDelegate::paintItemIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const auto &icon = index.data(Qt::DecorationRole).value<QIcon>();
    if (icon.isNull())
        return;

    QStyleOptionViewItem opt = option;

    // draw icon
    QRect iconRect = opt.rect.adjusted(kIconLeftPadding, 0, 0, 0);
    iconRect.setSize({ kIconWidth, kIconHeight });
    iconRect.moveTop(iconRect.top() + (opt.rect.bottom() - iconRect.bottom()) / 2);

    const auto &px = createCustomOpacityPixmap(icon.pixmap(iconRect.size()), 0.4f);
    painter->drawPixmap(iconRect, px);
}

QPixmap CompleterViewDelegate::createCustomOpacityPixmap(const QPixmap &px, float opacity) const
{
    QPixmap tmp(px.size());
    tmp.fill(Qt::transparent);

    QPainter p(&tmp);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawPixmap(0, 0, px);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(px.rect(), QColor(0, 0, 0, static_cast<int>(255 * opacity)));
    p.end();

    return tmp;
}
