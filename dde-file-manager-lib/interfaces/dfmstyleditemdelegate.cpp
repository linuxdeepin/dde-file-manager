/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dfmstyleditemdelegate.h"
#include "dfileviewhelper.h"
#include "private/dstyleditemdelegate_p.h"

#include <QDebug>
#include <QAbstractItemView>
#include <QPainter>
#include <QPainterPath>
#include <QGuiApplication>
#include <QThreadStorage>

DFMStyledItemDelegate::DFMStyledItemDelegate(DFileViewHelper *parent)
    : DFMStyledItemDelegate(*new DFMStyledItemDelegatePrivate(this), parent)
{

}

DFMStyledItemDelegate::~DFMStyledItemDelegate()
{

}

DFileViewHelper *DFMStyledItemDelegate::parent() const
{
    return dynamic_cast<DFileViewHelper *>(QStyledItemDelegate::parent());
}

QModelIndex DFMStyledItemDelegate::editingIndex() const
{
    Q_D(const DFMStyledItemDelegate);

    return d->editingIndex;
}

QWidget *DFMStyledItemDelegate::editingIndexWidget() const
{
    Q_D(const DFMStyledItemDelegate);

    return parent()->indexWidget(d->editingIndex);
}

QSize DFMStyledItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    Q_D(const DFMStyledItemDelegate);

    return d->itemSizeHint;
}

void DFMStyledItemDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    Q_D(const DFMStyledItemDelegate);

    QStyledItemDelegate::destroyEditor(editor, index);

    d->editingIndex = QModelIndex();
}

QString DFMStyledItemDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    if (value.type() == QVariant::String)
        return value.toString();

    return QStyledItemDelegate::displayText(value, locale);
}

QModelIndexList DFMStyledItemDelegate::hasWidgetIndexs() const
{
    Q_D(const DFMStyledItemDelegate);

    if (!d->editingIndex.isValid())
        return QModelIndexList();

    return QModelIndexList() << d->editingIndex;
}

void DFMStyledItemDelegate::hideAllIIndexWidget()
{
    Q_D(const DFMStyledItemDelegate);

    hideNotEditingIndexWidget();

    if (d->editingIndex.isValid()) {
        parent()->setIndexWidget(d->editingIndex, 0);

        d->editingIndex = QModelIndex();
    }
}

void DFMStyledItemDelegate::hideNotEditingIndexWidget()
{

}

void DFMStyledItemDelegate::commitDataAndCloseActiveEditor()
{
    Q_D(const DFMStyledItemDelegate);

    QWidget *editor = parent()->indexWidget(d->editingIndex);

    if (!editor)
        return;

    QMetaObject::invokeMethod(this, "_q_commitDataAndCloseEditor",
                              Qt::DirectConnection, Q_ARG(QWidget *, editor));
}

QRect DFMStyledItemDelegate::fileNameRect(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QList<QRect> &rects = paintGeomertys(option, index);

    if (rects.count() > 1)
        return rects.at(1);

    return QRect();
}
int DFMStyledItemDelegate::iconSizeLevel() const
{
    return -1;
}

int DFMStyledItemDelegate::minimumIconSizeLevel() const
{
    return -1;
}

int DFMStyledItemDelegate::maximumIconSizeLevel() const
{
    return -1;
}

int DFMStyledItemDelegate::increaseIcon()
{
    return -1;
}

int DFMStyledItemDelegate::decreaseIcon()
{
    return -1;
}

int DFMStyledItemDelegate::setIconSizeByIconSizeLevel(int level)
{
    Q_UNUSED(level)

    return -1;
}

QList<QRectF> DFMStyledItemDelegate::drawText(const QModelIndex &index, QPainter *painter, QTextLayout *layout, const QRectF &boundingRect,
                                              qreal radius, const QBrush &background, QTextOption::WrapMode wordWrap,
                                              Qt::TextElideMode mode, int flags, const QColor &shadowColor) const
{
    initTextLayout(index, layout);

    QList<QRectF> boundingRegion;
    DFMGlobal::elideText(layout, boundingRect.size(), wordWrap, mode, d_func()->textLineHeight, flags, 0,
                         painter, boundingRect.topLeft(), shadowColor, QPointF(0, 1),
                         background, radius, &boundingRegion);

    return boundingRegion;
}

QList<QRectF> DFMStyledItemDelegate::drawText(const QModelIndex &index, QPainter *painter, const QString &text, const QRectF &boundingRect,
                                              qreal radius, const QBrush &background, QTextOption::WrapMode wordWrap,
                                              Qt::TextElideMode mode, int flags, const QColor &shadowColor) const
{
    QTextLayout layout;

    layout.setText(text);

    if (painter)
        layout.setFont(painter->font());

    return drawText(index, painter, &layout, boundingRect, radius, background, wordWrap, mode, flags, shadowColor);
}

DFMStyledItemDelegate::DFMStyledItemDelegate(DFMStyledItemDelegatePrivate &dd, DFileViewHelper *parent)
    : QStyledItemDelegate(parent)
    , d_ptr(&dd)
{
    dd.init();
}

void DFMStyledItemDelegate::initTextLayout(const QModelIndex &index, QTextLayout *layout) const
{
    Q_UNUSED(index)
    Q_UNUSED(layout)
}

void DFMStyledItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    parent()->initStyleOption(option, index);
}

QList<QRectF> DFMStyledItemDelegate::getCornerGeometryList(const QRectF &baseRect, const QSizeF &cornerSize) const
{
    QList<QRectF> list;
    int offset = baseRect.width() / 8;
    const QSizeF &offset_size = cornerSize / 2;

    list << QRectF(QPointF(baseRect.right() - offset - offset_size.width(),
                           baseRect.bottom() - offset - offset_size.height()), cornerSize);
    list << QRectF(QPointF(baseRect.left() + offset - offset_size.width(), list.first().top()), cornerSize);
    list << QRectF(QPointF(list.at(1).left(), baseRect.top() + offset - offset_size.height()), cornerSize);
    list << QRectF(QPointF(list.first().left(), list.at(2).top()), cornerSize);

    return list;
}

QPixmap DFMStyledItemDelegate::getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio = 1.0, QIcon::Mode mode, QIcon::State state)
{
    // ###(zccrs): 开启Qt::AA_UseHighDpiPixmaps后，QIcon::pixmap会自动执行 pixmapSize *= qApp->devicePixelRatio()
    //             而且，在有些QIconEngine的实现中，会去调用另一个QIcon::pixmap，导致 pixmapSize 在这种嵌套调用中越来越大
    //             最终会获取到一个是期望大小几倍的图片，由于图片太大，会很快将 QPixmapCache 塞满，导致后面再调用QIcon::pixmap
    //             读取新的图片时无法缓存，非常影响图片绘制性能。此处在获取图片前禁用 Qt::AA_UseHighDpiPixmaps，自行处理图片大小问题
    bool useHighDpiPixmaps = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps);
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, false);

    if (icon.isNull())
        return QPixmap();

    QSize icon_size = icon.actualSize(size, mode, state);
    //取出icon的真实大小
    QList<QSize> iconSizeList = icon.availableSizes();
    QSize iconRealSize;
    if (iconSizeList.count() > 0)
        iconRealSize = iconSizeList.first();
    else
        iconRealSize = icon_size;
    if (iconRealSize.width() <= 0 || iconRealSize.height() <= 0) {
//        return icon.pixmap(iconRealSize);
        return icon.pixmap(icon_size);
    }

    //确保特殊比例icon的高或宽不为0
    bool isSpecialSize = false;
    QSize tempSize(size.width(), size.height());
    while (icon_size.width() < 1) {
        tempSize.setHeight(tempSize.height() * 2);
        icon_size = icon.actualSize(tempSize, mode, state);
        isSpecialSize = true;
    }
    while (icon_size.height() < 1) {
        tempSize.setWidth(tempSize.width() * 2);
        icon_size = icon.actualSize(tempSize, mode, state);
        isSpecialSize = true;
    }

    if ((icon_size.width() > size.width() || icon_size.height() > size.height()) && !isSpecialSize)
        icon_size.scale(size, Qt::KeepAspectRatio);

    QSize pixmapSize = icon_size * pixelRatio;
    QPixmap px = icon.pixmap(pixmapSize, mode, state);

    // restore the value
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps, useHighDpiPixmaps);

//    if (icon_size.width() == 0) {
//        icon_size.setWidth(1);
//    }
//    if (icon_size.height() == 0) {
//        icon_size.setHeight(1);
//    }

//    icon_size = icon_size * pixelRatio;
//    px = px.scaled(icon_size.width(), icon_size.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    //约束特殊比例icon的尺寸
    if (isSpecialSize) {
        if (px.width() > size.width() * pixelRatio) {
            px = px.scaled(size.width() * pixelRatio, px.height(), Qt::IgnoreAspectRatio);
        } else if (px.height() > size.height() * pixelRatio) {
            px = px.scaled(px.width(), size.height() * pixelRatio, Qt::IgnoreAspectRatio);
        }
    }

    if (px.width() > icon_size.width() * pixelRatio) {
        px.setDevicePixelRatio(px.width() / (qreal)icon_size.width());
    } else if (px.height() > icon_size.height() * pixelRatio) {
        px.setDevicePixelRatio(px.height() / (qreal)icon_size.height());
    } else {
        px.setDevicePixelRatio(pixelRatio);
    }

    return px;
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

namespace DEEPIN_QT_THEME {
QThreadStorage<QString> colorScheme;
void(*setFollowColorScheme)(bool) = nullptr;
bool(*followColorScheme)() = nullptr;
}

void DFMStyledItemDelegate::paintIcon(QPainter *painter, const QIcon &icon, const QRectF &rect, Qt::Alignment alignment, QIcon::Mode mode, QIcon::State state)
{
    if (DEEPIN_QT_THEME::followColorScheme
            && (*DEEPIN_QT_THEME::followColorScheme)()
            && painter->device()->devType() == QInternal::Widget) {
        const QWidget *widget = dynamic_cast<QWidget *>(painter->device());
        const QPalette &pal = widget->palette();
        DEEPIN_QT_THEME::colorScheme.setLocalData(mode == QIcon::Selected ? pal.highlightedText().color().name() : pal.windowText().color().name());
    }

    // Copy of QStyle::alignedRect
    alignment = visualAlignment(painter->layoutDirection(), alignment);
    const qreal pixel_ratio = painter->device()->devicePixelRatioF();
    const QPixmap &px = getIconPixmap(icon, rect.size().toSize(), pixel_ratio, mode, state);
    qreal x = rect.x();
    qreal y = rect.y();
    qreal w = px.width() / px.devicePixelRatio();
    qreal h = px.height() / px.devicePixelRatio();
    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += (rect.size().height() - h) / 2.0;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += rect.size().height() - h;
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += rect.size().width() - w;
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += (rect.size().width() - w) / 2.0;

    painter->drawPixmap(qRound(x), qRound(y), px);
}

void DFMStyledItemDelegate::paintCircleList(QPainter *painter, QRectF boundingRect, qreal diameter, const QList<QColor> &colors, const QColor &borderColor)
{
    bool antialiasing = painter->testRenderHint(QPainter::Antialiasing);
    const QPen pen = painter->pen();

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(borderColor, 1));

    for (const QColor &color : colors) {
        QPainterPath circle;

        circle.addEllipse(QRectF(QPointF(boundingRect.right() - diameter, boundingRect.top()), boundingRect.bottomRight()));
        painter->fillPath(circle, color);
        painter->drawPath(circle);
        boundingRect.setRight(boundingRect.right() - diameter / 2);
    }

    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing, antialiasing);
}

void DFMStyledItemDelegatePrivate::init()
{
    Q_Q(DFMStyledItemDelegate);

    q->connect(q, &DFMStyledItemDelegate::commitData, q->parent(), &DFileViewHelper::handleCommitData);
    q->connect(q->parent()->parent(), &QAbstractItemView::iconSizeChanged, q, &DFMStyledItemDelegate::updateItemSizeHint);

    QAbstractItemModel *model = q->parent()->parent()->model();
    Q_ASSERT(model);

    q->connect(model, SIGNAL(rowsInserted(QModelIndex, int, int)), SLOT(_q_onRowsInserted(QModelIndex, int, int)));
    q->connect(model, SIGNAL(rowsRemoved(QModelIndex, int, int)), SLOT(_q_onRowsRemoved(QModelIndex, int, int)));

    textLineHeight = q->parent()->parent()->fontMetrics().height();
}

void DFMStyledItemDelegatePrivate::_q_onRowsInserted(const QModelIndex &parent, int first, int last)
{
    if (editingIndex.isValid() && first <= editingIndex.row() && !editingIndex.parent().isValid()) {
        editingIndex = parent.child(editingIndex.row() + last - first + 1, editingIndex.column());
    }
}

void DFMStyledItemDelegatePrivate::_q_onRowsRemoved(const QModelIndex &parent, int first, int last)
{
    if (editingIndex.isValid() && first <= editingIndex.row() && !editingIndex.parent().isValid()) {
        editingIndex = parent.child(editingIndex.row() - last + first - 1, editingIndex.column());
    }
}

#include "moc_dfmstyleditemdelegate.cpp"
