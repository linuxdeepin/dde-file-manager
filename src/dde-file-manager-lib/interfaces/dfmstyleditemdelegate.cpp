// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmstyleditemdelegate.h"
#include "dfileviewhelper.h"
#include "dfilesystemmodel.h"
#include "pixmapiconextend.h"
#include "private/dstyleditemdelegate_p.h"

#include <QDebug>
#include <QAbstractItemView>
#include <QPainter>
#include <QPainterPath>
#include <QGuiApplication>
#include <QThreadStorage>
#include <QPainterPath>

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
        parent()->setIndexWidget(d->editingIndex, nullptr);

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

    //根据实际文件名所在列返回rect
    const QList<int> roleList = parent()->columnRoleList();
    int fileNameIndex = 0;
    for (int i = 0; i < roleList.length(); ++i) {
        if (roleList.at(i) == DFileSystemModel::FileDisplayNameRole || roleList.at(i) == DFileSystemModel::FileNameRole) {
            fileNameIndex = i;
            break;
        }
    }

    //第0个是icon的rect
    if (rects.count() > 1)
        return rects.at(fileNameIndex + 1);

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
    DFMGlobal::elideText(layout, boundingRect.size(), wordWrap, mode, d_func()->textLineHeight, flags, nullptr,
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
    : QStyledItemDelegate(parent), d_ptr(&dd)
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
    double offset = baseRect.width() / 8;
    const QSizeF &offset_size = cornerSize / 2;

    list.append(QRectF(QPointF(baseRect.right() - offset - offset_size.width(),
                               baseRect.bottom() - offset - offset_size.height()),
                       cornerSize));
    list.append(QRectF(QPointF(baseRect.left() + offset - offset_size.width(), list.first().top()), cornerSize));
    list.append(QRectF(QPointF(list.at(1).left(), baseRect.top() + offset - offset_size.height()), cornerSize));
    list.append(QRectF(QPointF(list.first().left(), list.at(2).top()), cornerSize));

    return list;
}

QPixmap DFMStyledItemDelegate::getIconPixmap(const QIcon &icon, const QSize &size, qreal pixelRatio = 1.0, QIcon::Mode mode, QIcon::State state)
{
    if (icon.isNull())
        return QPixmap();

    //确保当前参数参入获取图片大小大于0
    if (size.width() <= 0 || size.height() <= 0)
        return QPixmap();

    return PixmapIconExtend(icon).pixmapExtend(size, pixelRatio, mode, state);
}

void DFMStyledItemDelegate::paintDragIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QSize &size) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QRectF icon_rect = opt.rect;
    icon_rect.setSize(size);

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    paintIcon(painter, opt.icon, icon_rect, Qt::AlignCenter, QIcon::Normal);
}

QSize DFMStyledItemDelegate::getIndexIconSize(const QStyleOptionViewItem &option, const QModelIndex &index, const QSize &size) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QRectF icon_rect = opt.rect;
    icon_rect.setSize(size);

    QSize iconSize = opt.icon.actualSize(icon_rect.size().toSize(), QIcon::Normal, QIcon::Off);
    if (iconSize.width() > size.width() || iconSize.height() > size.height())
        iconSize.scale(size, Qt::KeepAspectRatio);

    return iconSize;
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

void DFMStyledItemDelegate::paintIcon(QPainter *painter, const QIcon &icon, const QRectF &rect, Qt::Alignment alignment, QIcon::Mode mode, QIcon::State state)
{
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
    const QBrush brush = painter->brush();

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(borderColor, 1));

    for (const QColor &color : colors) {
        QPainterPath circle;

        //根据tag颜色设置笔刷
        painter->setBrush(QBrush(color));
        circle.addEllipse(QRectF(QPointF(boundingRect.right() - diameter, boundingRect.top()), boundingRect.bottomRight()));
        //        painter->fillPath(circle, color);
        painter->drawPath(circle);
        boundingRect.setRight(boundingRect.right() - diameter / 2);
    }

    painter->setPen(pen);
    painter->setBrush(brush);
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

    textLineHeight = q->parent()->parent()->fontMetrics().lineSpacing();
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
