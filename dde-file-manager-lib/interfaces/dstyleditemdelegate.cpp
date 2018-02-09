/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dstyleditemdelegate.h"
#include "dfileviewhelper.h"
#include "private/dstyleditemdelegate_p.h"

#include <QDebug>
#include <QAbstractItemView>
#include <QPainter>
#include <QGuiApplication>

DStyledItemDelegate::DStyledItemDelegate(DFileViewHelper *parent)
    : DStyledItemDelegate(*new DStyledItemDelegatePrivate(this), parent)
{

}

DStyledItemDelegate::~DStyledItemDelegate()
{

}

DFileViewHelper *DStyledItemDelegate::parent() const
{
    return static_cast<DFileViewHelper*>(QStyledItemDelegate::parent());
}

QModelIndex DStyledItemDelegate::editingIndex() const
{
    Q_D(const DStyledItemDelegate);

    return d->editingIndex;
}

QWidget *DStyledItemDelegate::editingIndexWidget() const
{
    Q_D(const DStyledItemDelegate);

    return parent()->indexWidget(d->editingIndex);
}

QSize DStyledItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    Q_D(const DStyledItemDelegate);

    return d->itemSizeHint;
}

void DStyledItemDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    Q_D(const DStyledItemDelegate);

    QStyledItemDelegate::destroyEditor(editor, index);

    d->editingIndex = QModelIndex();
}

QString DStyledItemDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    if (value.type() == QVariant::String)
        return value.toString();

    return QStyledItemDelegate::displayText(value, locale);
}

QModelIndexList DStyledItemDelegate::hasWidgetIndexs() const
{
    Q_D(const DStyledItemDelegate);

    if (!d->editingIndex.isValid())
        return QModelIndexList();

    return QModelIndexList() << d->editingIndex;
}

void DStyledItemDelegate::hideAllIIndexWidget()
{
    Q_D(const DStyledItemDelegate);

    hideNotEditingIndexWidget();

    if (d->editingIndex.isValid()) {
        parent()->setIndexWidget(d->editingIndex, 0);

        d->editingIndex = QModelIndex();
    }
}

void DStyledItemDelegate::hideNotEditingIndexWidget()
{

}

void DStyledItemDelegate::commitDataAndCloseActiveEditor()
{
    Q_D(const DStyledItemDelegate);

    QWidget *editor = parent()->indexWidget(d->editingIndex);

    if (!editor)
        return;

    QMetaObject::invokeMethod(this, "_q_commitDataAndCloseEditor",
                              Qt::DirectConnection, Q_ARG(QWidget*, editor));
}

QRect DStyledItemDelegate::fileNameRect(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QList<QRect> &rects = paintGeomertys(option, index);

    if (rects.count() > 1)
        return rects.at(1);

    return QRect();
}
int DStyledItemDelegate::iconSizeLevel() const
{
    return -1;
}

int DStyledItemDelegate::minimumIconSizeLevel() const
{
    return -1;
}

int DStyledItemDelegate::maximumIconSizeLevel() const
{
    return -1;
}

int DStyledItemDelegate::increaseIcon()
{
    return -1;
}

int DStyledItemDelegate::decreaseIcon()
{
    return -1;
}

int DStyledItemDelegate::setIconSizeByIconSizeLevel(int level)
{
    Q_UNUSED(level)

    return -1;
}

DStyledItemDelegate::DStyledItemDelegate(DStyledItemDelegatePrivate &dd, DFileViewHelper *parent)
    : QStyledItemDelegate(parent)
    , d_ptr(&dd)
{
    dd.init();
}

void DStyledItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    parent()->initStyleOption(option, index);
}

QList<QRect> DStyledItemDelegate::getCornerGeometryList(const QRect &baseRect, const QSize &cornerSize) const
{
    QList<QRect> list;
    int offset = baseRect.width() / 8;
    const QSize &offset_size = cornerSize / 2;

    list << QRect(QPoint(baseRect.right() - offset - offset_size.width(),
                         baseRect.bottom() - offset - offset_size.height()), cornerSize);
    list << QRect(QPoint(baseRect.left() + offset - offset_size.width(), list.first().top()), cornerSize);
    list << QRect(QPoint(list.at(1).left(), baseRect.top() + offset - offset_size.height()), cornerSize);
    list << QRect(QPoint(list.first().left(), list.at(2).top()), cornerSize);

    return list;
}

static inline Qt::Alignment visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment)
{
    if (!(alignment & Qt::AlignHorizontal_Mask))
        alignment |= Qt::AlignLeft;
    if (!(alignment & Qt::AlignAbsolute) && (alignment & (Qt::AlignLeft | Qt::AlignRight))) {
        if (direction == Qt::RightToLeft)
            alignment ^= (Qt::AlignLeft | Qt::AlignRight);
        alignment |= Qt::AlignAbsolute;
    }
    return alignment;
}

void DStyledItemDelegate::paintIcon(QPainter *painter, const QIcon &icon, const QRect &rect, Qt::Alignment alignment, QIcon::Mode mode, QIcon::State state)
{
    // Copy of QStyle::alignedRect
    QSize size = icon.actualSize(rect.size(), mode, state);

    if (size.width() > rect.width() || size.height() > rect.height())
        size.scale(rect.size(), Qt::KeepAspectRatio);

    alignment = visualAlignment(painter->layoutDirection(), alignment);
    const qreal pixel_ratio = painter->device()->devicePixelRatioF();
    int x = rect.x();
    int y = rect.y();
    int w = size.width();
    int h = size.height();
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += rect.size().height()/2 - h/2;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += rect.size().height() - h;
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += rect.size().width() - w;
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += rect.size().width()/2 - w/2;
    QRect alignedRect(x, y, w, h);

    QSize pixmapSize = alignedRect.size() * pixel_ratio;
    QPixmap px = icon.pixmap(pixmapSize, mode, state);

    if (px.width() > alignedRect.width() * pixel_ratio || px.height() > alignedRect.height() * pixel_ratio)
        px = px.scaled(alignedRect.size() * pixel_ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    px.setDevicePixelRatio(pixel_ratio);
    painter->drawPixmap(alignedRect.topLeft(), px);
}

void DStyledItemDelegatePrivate::init()
{
    Q_Q(DStyledItemDelegate);

    q->connect(q, &DStyledItemDelegate::commitData, q->parent(), &DFileViewHelper::handleCommitData);
    q->connect(q->parent()->parent(), &QAbstractItemView::iconSizeChanged, q, &DStyledItemDelegate::updateItemSizeHint);

    QAbstractItemModel *model = q->parent()->parent()->model();
    Q_ASSERT(model);

    q->connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(_q_onRowsInserted(QModelIndex,int,int)));
    q->connect(model, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(_q_onRowsRemoved(QModelIndex,int,int)));

    textLineHeight = q->parent()->parent()->fontMetrics().height();
}

void DStyledItemDelegatePrivate::_q_onRowsInserted(const QModelIndex &parent, int first, int last)
{
    if (editingIndex.isValid() && first <= editingIndex.row() && !editingIndex.parent().isValid()) {
        editingIndex = parent.child(editingIndex.row() + last - first + 1, editingIndex.column());
    }
}

void DStyledItemDelegatePrivate::_q_onRowsRemoved(const QModelIndex &parent, int first, int last)
{
    if (editingIndex.isValid() && first <= editingIndex.row() && !editingIndex.parent().isValid()) {
        editingIndex = parent.child(editingIndex.row() - last + first - 1, editingIndex.column());
    }
}

#include "moc_dstyleditemdelegate.cpp"
