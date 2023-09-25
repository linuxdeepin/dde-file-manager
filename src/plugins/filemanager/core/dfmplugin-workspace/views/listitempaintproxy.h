// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LISTITEMPAINTPROXY_H
#define LISTITEMPAINTPROXY_H

#include "abstractitempaintproxy.h"

namespace dfmplugin_workspace {

class ListItemPaintProxy : public AbstractItemPaintProxy
{
public:
    explicit ListItemPaintProxy(QObject *parent = nullptr);

    virtual void drawIcon(QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

}

#endif // LISTITEMPAINTPROXY_H
