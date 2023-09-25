// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREEITEMPAINTPROXY_H
#define TREEITEMPAINTPROXY_H

#include "abstractitempaintproxy.h"

namespace dfmplugin_workspace {

class FileView;
class TreeItemPaintProxy : public AbstractItemPaintProxy
{
public:
    explicit TreeItemPaintProxy(QObject *parent = nullptr);

    virtual void drawIcon(QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    QRectF iconRect(const QModelIndex &index, const QRect &itemRect);

private:
    QRect drawExpandArrow(QPainter *painter, const QRectF &rect, const QStyleOptionViewItem &option, const QModelIndex &index);
    FileView *view();
};

}

#endif // TREEITEMPAINTPROXY_H
