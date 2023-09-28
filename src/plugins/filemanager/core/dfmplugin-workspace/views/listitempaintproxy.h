// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LISTITEMPAINTPROXY_H
#define LISTITEMPAINTPROXY_H

#include "abstractitempaintproxy.h"

namespace dfmplugin_workspace {

class FileView;
class ListItemPaintProxy : public AbstractItemPaintProxy
{
public:
    explicit ListItemPaintProxy(QObject *parent = nullptr);

    virtual void drawIcon(QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    virtual QRectF rectByType(RectOfItemType type, const QModelIndex &index) override;
    virtual QList<QRect> allPaintRect(const QStyleOptionViewItem &option, const QModelIndex &index) override;

    QRectF iconRect(const QModelIndex &index, const QRect &itemRect);
private:
    FileView *view();
};

}

#endif // LISTITEMPAINTPROXY_H
