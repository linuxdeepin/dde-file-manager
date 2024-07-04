// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "expandedItem.h"
#include "iconitemdelegate.h"
#include "utils/itemdelegatehelper.h"
#include "utils/fileviewhelper.h"
#include "views/fileview.h"
#include "views/private/delegatecommon.h"
#include "models/fileviewmodel.h"
#include "events/workspaceeventsequence.h"

#include <dfm-base/dfm_global_defines.h>

#include <QPainter>
#include <QDebug>

#include <cmath>

using namespace dfmplugin_workspace;
using namespace dfmbase;

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

    if (option.text.isEmpty())
        return;

    const QMargins &margins = contentsMargins();

    QRect labelRect(kIconModeRectRadius + margins.left(),
                    iconHeight + kIconModeTextPadding + 2 * kIconModeIconSpacing + margins.top(),
                    width() - kIconModeRectRadius * 2,
                    INT_MAX);

    QString str = delegate->displayFileName(index);

    QScopedPointer<ElideTextLayout> layout(ItemDelegateHelper::createTextLayout(str, QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                                                pa.fontMetrics().height(), Qt::AlignCenter, &pa));
    layout->setAttribute(ElideTextLayout::kBackgroundRadius, kIconModeRectRadius);

    const FileInfoPointer &info = delegate->parent()->parent()->model()->fileInfo(index);
    if (!info)
        return;

    WorkspaceEventSequence::instance()->doIconItemLayoutText(info, layout.data());
    const QList<QRectF> lines = layout->layout(labelRect, option.textElideMode, &pa, option.palette.brush(QPalette::Normal, QPalette::Highlight));

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
                        iconHeight + kIconModeTextPadding + 2 * kIconModeIconSpacing + margins.top(),
                        width - kIconModeRectRadius * 2,
                        INT_MAX);

        QString str = delegate->displayFileName(index);
        const QList<QRectF> &lines = delegate->calFileNameRect(str, labelRect, option.textElideMode);

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
