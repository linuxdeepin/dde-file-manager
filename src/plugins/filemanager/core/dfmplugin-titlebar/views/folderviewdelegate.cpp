// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "folderviewdelegate.h"

#include <QPainter>
#include <QCompleter>
#include <QApplication>
#include <QHelpEvent>
#include <QToolTip>

using namespace dfmplugin_titlebar;
DWIDGET_USE_NAMESPACE

FolderViewDelegate::FolderViewDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
}

void FolderViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
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
        painter->save();
        painter->setBrush(option.palette.brush(cg, QPalette::Highlight));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(option.rect, 8, 8);
        painter->restore();
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
    QRect textRect = option.rect.adjusted(kTextLeftPadding, 0, 0, 0);
    QString elidedText = option.fontMetrics.elidedText(text, Qt::ElideRight, textRect.width());
    painter->drawText(textRect, Qt::AlignVCenter, elidedText);
}

QSize FolderViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize s = QStyledItemDelegate::sizeHint(option, index);
    s.setHeight(kFolderItemHeight);

    return s;
}

bool FolderViewDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() != QEvent::ToolTip) {
        return QStyledItemDelegate::helpEvent(event, view, option, index);
    }
    hideTooltipImmediately();
    auto text = index.data(Qt::DisplayRole).toString();
    if (text.contains('\n'))
        text = text.replace('\n', ' ');
    int minWidth = qMin(view->contentsRect().width(), option.rect.width());
    minWidth = minWidth - 2 * kItemMargin;
    QRect textRect = option.rect;
    textRect.setWidth(minWidth);
    textRect = textRect.adjusted(kTextLeftPadding, 0, 0, 0);
    QString elidedText = option.fontMetrics.elidedText(text, Qt::ElideRight, textRect.width());
    if (elidedText != text) {
        QToolTip::showText(event->globalPos(), text, view);
    }
    return true;
}

void FolderViewDelegate::paintItemIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const auto &icon = index.data(Qt::DecorationRole).value<QIcon>();
    if (icon.isNull())
        return;

    QStyleOptionViewItem opt = option;

    // draw icon
    QRect iconRect = opt.rect.adjusted(kIconLeftPadding, 0, 0, 0);
    iconRect.setSize({ kFolderIconSize, kFolderIconSize });
    iconRect.moveTop(iconRect.top() + (opt.rect.bottom() - iconRect.bottom()) / 2);

    const auto &px = createCustomOpacityPixmap(icon.pixmap(iconRect.size()), 1.0f);
    painter->drawPixmap(iconRect, px);
}

QPixmap FolderViewDelegate::createCustomOpacityPixmap(const QPixmap &px, float opacity) const
{
    QPixmap tmp(px.size());
    tmp.setDevicePixelRatio(qApp->devicePixelRatio());
    tmp.fill(Qt::transparent);

    QPainter p(&tmp);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawPixmap(0, 0, px);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(px.rect(), QColor(0, 0, 0, static_cast<int>(255 * opacity)));
    p.end();

    return tmp;
}

void FolderViewDelegate::hideTooltipImmediately()
{
    QWidgetList qwl = QApplication::topLevelWidgets();
    for (QWidget *qw : qwl) {
        if (QStringLiteral("QTipLabel") == qw->metaObject()->className()) {
            qw->close();
        }
    }
}
