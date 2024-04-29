// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "listitempaintproxy.h"
#include "fileview.h"
#include "utils/itemdelegatehelper.h"

using namespace dfmplugin_workspace;

ListItemPaintProxy::ListItemPaintProxy(QObject *parent)
    : AbstractItemPaintProxy(parent)
{
}

void ListItemPaintProxy::drawIcon(QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(index)

    *rect = iconRect(index, rect->toRect());

    bool isEnabled = option.state & QStyle::State_Enabled;
    ItemDelegateHelper::paintIcon(painter, option.icon, { *rect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled });
}

QRectF ListItemPaintProxy::rectByType(RectOfItemType type, const QModelIndex &index)
{
    QRect itemRect = view()->visualRect(index);
    switch (type) {
    case RectOfItemType::kItemIconRect:
        return iconRect(index, itemRect);
    default:
        return QRectF();
    }
}

QList<QRect> ListItemPaintProxy::allPaintRect(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QList<QRect> rects {};

    QRect itemRect = option.rect;
    rects.append(iconRect(index, itemRect).toRect());

    return rects;
}

QRectF ListItemPaintProxy::iconRect(const QModelIndex &index, const QRect &itemRect)
{
    Q_UNUSED(index)

    QRectF iconRect = itemRect;
    QSize iconSize = view()->iconSize();
    iconRect.setSize(iconSize);

    iconRect.moveLeft(iconRect.left() + kListModeLeftMargin + +kListModeLeftPadding);
    iconRect.moveTop(iconRect.top() + ((itemRect.bottom() - iconRect.bottom()) / 2));

    return iconRect;
}

FileView *ListItemPaintProxy::view()
{
    return qobject_cast<FileView *>(parent());
}
