/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "baseitemdelegate.h"
#include "private/baseitemdelegate_p.h"
#include "fileview.h"
#include "utils/fileviewhelper.h"
#include "utils/itemdelegatehelper.h"
#include "events/workspaceeventcaller.h"
#include "models/filesortfilterproxymodel.h"

#include <QTextLayout>
#include <QPainter>

DPWORKSPACE_USE_NAMESPACE

BaseItemDelegate::BaseItemDelegate(FileViewHelper *parent)
    : BaseItemDelegate(*new BaseItemDelegatePrivate(this), parent)
{
}

BaseItemDelegate::BaseItemDelegate(BaseItemDelegatePrivate &dd, FileViewHelper *parent)
    : QStyledItemDelegate(parent),
      d(&dd)
{
    dd.init();
}

void BaseItemDelegate::initTextLayout(const QModelIndex &index, QTextLayout *layout) const
{
    Q_UNUSED(index);
    Q_UNUSED(layout);
}

void BaseItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    parent()->initStyleOption(option, index);
}

QList<QRectF> BaseItemDelegate::getCornerGeometryList(const QRectF &baseRect, const QSizeF &cornerSize) const
{
    QList<QRectF> list;
    double offset = baseRect.width() / 8;
    const QSizeF &offsetSize = cornerSize / 2;

    list.append(QRectF(QPointF(baseRect.right() - offset - offsetSize.width(),
                               baseRect.bottom() - offset - offsetSize.height()),
                       cornerSize));
    list.append(QRectF(QPointF(baseRect.left() + offset - offsetSize.width(), list.first().top()), cornerSize));
    list.append(QRectF(QPointF(list.at(1).left(), baseRect.top() + offset - offsetSize.height()), cornerSize));
    list.append(QRectF(QPointF(list.first().left(), list.at(2).top()), cornerSize));

    return list;
}

void BaseItemDelegate::paintEmblems(QPainter *painter, const QRectF &iconRect, const QModelIndex &index) const
{
    const QUrl &url = parent()->parent()->model()->getUrlByIndex(index);
    WorkspaceEventCaller::sendPaintEmblems(painter, iconRect, url);
}

BaseItemDelegate::~BaseItemDelegate()
{
}

QSize BaseItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    Q_D(const BaseItemDelegate);

    return d->itemSizeHint;
}

int BaseItemDelegate::iconSizeLevel() const
{
    return -1;
}

int BaseItemDelegate::minimumIconSizeLevel() const
{
    return -1;
}

int BaseItemDelegate::maximumIconSizeLevel() const
{
    return -1;
}

int BaseItemDelegate::increaseIcon()
{
    return -1;
}

int BaseItemDelegate::decreaseIcon()
{
    return -1;
}

int BaseItemDelegate::setIconSizeByIconSizeLevel(int level)
{
    Q_UNUSED(level)
    return -1;
}

QModelIndexList BaseItemDelegate::hasWidgetIndexs() const
{
    Q_D(const BaseItemDelegate);

    if (!d->editingIndex.isValid())
        return QModelIndexList();
    return QModelIndexList() << d->editingIndex;
}

void BaseItemDelegate::hideAllIIndexWidget()
{
    hideNotEditingIndexWidget();

    if (d->editingIndex.isValid()) {
        parent()->parent()->setIndexWidget(d->editingIndex, nullptr);

        d->editingIndex = QModelIndex();
    }
}

void BaseItemDelegate::hideNotEditingIndexWidget()
{
}

void BaseItemDelegate::commitDataAndCloseActiveEditor()
{
    QWidget *editor = parent()->indexWidget(d->editingIndex);

    if (!editor)
        return;

    QMetaObject::invokeMethod(this, "_q_commitDataAndCloseEditor",
                              Qt::DirectConnection, Q_ARG(QWidget *, editor));
}

QList<QRectF> BaseItemDelegate::drawText(const QModelIndex &index, QPainter *painter, QTextLayout *layout, const QRectF &boundingRect, qreal radius, const QBrush &background, QTextOption::WrapMode wordWrap, Qt::TextElideMode mode, int flags, const QColor &shadowColor) const
{
    initTextLayout(index, layout);

    QList<QRectF> boundingRegion;

    ItemDelegateHelper::elideText(layout, boundingRect.size(), wordWrap, mode, d_func()->textLineHeight, flags, nullptr,
                                  painter, boundingRect.topLeft(), shadowColor, QPointF(0, 1),
                                  background, radius, &boundingRegion);
    return boundingRegion;
}

QList<QRectF> BaseItemDelegate::drawText(const QModelIndex &index, QPainter *painter, const QString &text, const QRectF &boundingRect, qreal radius, const QBrush &background, QTextOption::WrapMode wordWrap, Qt::TextElideMode mode, int flags, const QColor &shadowColor) const
{
    QTextLayout layout;

    layout.setText(text);

    if (painter)
        layout.setFont(painter->font());

    return drawText(index, painter, &layout, boundingRect, radius, background, wordWrap, mode, flags, shadowColor);
}

FileViewHelper *BaseItemDelegate::parent() const
{
    return dynamic_cast<FileViewHelper *>(QStyledItemDelegate::parent());
}

void BaseItemDelegate::paintDragIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QSize &size) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QRectF iconRect = opt.rect;
    iconRect.setSize(size);

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    ItemDelegateHelper::paintIcon(painter, opt.icon, iconRect, Qt::AlignCenter, QIcon::Normal);
}

QSize BaseItemDelegate::getIndexIconSize(const QStyleOptionViewItem &option, const QModelIndex &index, const QSize &size) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QRectF iconRect = opt.rect;
    iconRect.setSize(size);

    QSize iconSize = opt.icon.actualSize(iconRect.size().toSize(), QIcon::Normal, QIcon::Off);
    if (iconSize.width() > size.width() || iconSize.height() > size.height())
        iconSize.scale(size, Qt::KeepAspectRatio);

    return iconSize;
}

QModelIndex BaseItemDelegate::editingIndex() const
{
    Q_D(const BaseItemDelegate);

    return d->editingIndex;
}

QWidget *BaseItemDelegate::editingIndexWidget() const
{
    Q_D(const BaseItemDelegate);

    return parent()->indexWidget(d->editingIndex);
}
