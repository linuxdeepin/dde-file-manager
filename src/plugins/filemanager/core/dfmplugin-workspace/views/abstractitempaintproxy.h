// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTITEMPAINTPROXY_H
#define ABSTRACTITEMPAINTPROXY_H

#include "dfmplugin_workspace_global.h"

#include <QObject>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>

namespace dfmplugin_workspace {

class AbstractItemPaintProxy : public QObject
{
    Q_OBJECT
public:
    explicit AbstractItemPaintProxy(QObject *parent = nullptr);

    virtual void drawIcon(QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index);
    virtual void drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index);
    virtual void drawText(QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index);

    virtual QRectF rectByType(RectOfItemType type, const QModelIndex &index);
    virtual QList<QRect> allPaintRect(const QStyleOptionViewItem &option, const QModelIndex &index);

    virtual int iconRectIndex();

    void setStyleProxy(QStyle *style);

protected:
    QStyle *style;
};

}
#endif // ABSTRACTITEMPAINTPROXY_H
