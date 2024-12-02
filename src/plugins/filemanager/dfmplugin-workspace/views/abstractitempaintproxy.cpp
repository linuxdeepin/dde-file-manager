// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "abstractitempaintproxy.h"

using namespace dfmplugin_workspace;

AbstractItemPaintProxy::AbstractItemPaintProxy(QObject *parent)
    : QObject(parent)
{
}

void AbstractItemPaintProxy::drawIcon(QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(painter)
    Q_UNUSED(rect)
    Q_UNUSED(option)
    Q_UNUSED(index)
}

void AbstractItemPaintProxy::drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(index)
}

void AbstractItemPaintProxy::drawText(QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(painter)
    Q_UNUSED(rect)
    Q_UNUSED(option)
    Q_UNUSED(index)
}

QRectF AbstractItemPaintProxy::rectByType(RectOfItemType type, const QModelIndex &index)
{
    Q_UNUSED(type)
    Q_UNUSED(index)

    return QRectF();
}

QList<QRect> AbstractItemPaintProxy::allPaintRect(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    return QList<QRect>();
}

int AbstractItemPaintProxy::iconRectIndex()
{
    return 0;
}

void AbstractItemPaintProxy::setStyleProxy(QStyle *style)
{
    this->style = style;
}
