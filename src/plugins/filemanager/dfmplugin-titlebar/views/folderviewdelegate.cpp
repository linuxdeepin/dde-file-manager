// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "folderviewdelegate.h"

#include <DStyle>

#include <QPainter>
#include <QCompleter>
#include <QApplication>
#include <QHelpEvent>
#include <QToolTip>
#include <DPalette>
#include <DPaletteHelper>

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
    DPalette pl(DPaletteHelper::instance()->palette(option.widget));

    int radius = 12;
    if (auto view = dynamic_cast<QAbstractItemView *>(parent())) {
        radius = DStyle::pixelMetric(view->style(), DStyle::PM_FrameRadius, &opt, view);
    }

    bool isSelected = (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    bool isHovered = option.state & QStyle::State_MouseOver;

    if (isSelected || isHovered) {
        painter->save();
        painter->setPen(Qt::NoPen);

        if (isSelected) {
            QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                    ? QPalette::Normal
                    : QPalette::Disabled;
            if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
                cg = QPalette::Inactive;
            }
            painter->setBrush(option.palette.brush(cg, QPalette::Highlight));
        } else {
            QColor baseColor = pl.color(DPalette::Active, DPalette::ItemBackground);
            QColor hoverColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +10);
            painter->setBrush(hoverColor);
        }

        painter->drawRoundedRect(option.rect, radius, radius);
        painter->restore();
    }

    paintItemIcon(painter, option, index);

    if (isSelected) {
        painter->setPen(opt.palette.color(QPalette::Active, QPalette::HighlightedText));
    } else {
        painter->setPen(opt.palette.color(QPalette::Active, QPalette::Text));
    }

    painter->setFont(option.font);
    QString text = index.data(Qt::DisplayRole).toString();
    if (text.contains('\n')) {
        text = text.replace('\n', ' ');
    }

    QRect textRect = option.rect.adjusted(kTextLeftPadding, 0, 0, 0);
    if (option.fontMetrics.horizontalAdvance(text) > textRect.width()) {
        text = option.fontMetrics.elidedText(text, Qt::ElideMiddle, textRect.width());
    }

    painter->drawText(textRect, Qt::AlignVCenter, text);
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
    QRect textRect = option.rect;
    textRect = textRect.adjusted(kTextLeftPadding, 0, 0, 0);
    if (option.fontMetrics.horizontalAdvance(text) > textRect.width()) {
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

    // draw icon (avoid scaling to prevent blur, honor device pixel ratio)
    QRect iconRect = opt.rect.adjusted(kIconLeftPadding, 0, 0, 0);
    iconRect.setSize({ kFolderIconSize, kFolderIconSize });
    iconRect.moveTop(iconRect.top() + (opt.rect.bottom() - iconRect.bottom()) / 2);

    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF() : qApp->devicePixelRatio();
    QPixmap px = icon.pixmap(iconRect.size() * dpr);
    px.setDevicePixelRatio(dpr);

    // Draw at top-left without stretching to avoid resampling blur
    painter->drawPixmap(iconRect.topLeft(), px);
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
