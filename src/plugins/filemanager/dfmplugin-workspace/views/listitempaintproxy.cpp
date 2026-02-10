// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "listitempaintproxy.h"
#include "fileview.h"
#include "utils/itemdelegatehelper.h"
#include <dfm-base/dfm_global_defines.h>

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
    auto drawFileIcon = ItemDelegateHelper::paintIcon(painter, option.icon, { *rect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled, QIcon::Off, dfmbase::Global::ViewMode::kListMode,
                                                                              isThumnailIconIndex(index) });
    // If the thumbnail drawing is empty, then redraw the file fileicon
    if (!drawFileIcon) {
        const QIcon &fileIcon = index.data(dfmbase::Global::ItemRoles::kItemFileIconRole).value<QIcon>();
        ItemDelegateHelper::paintIcon(painter, fileIcon, { *rect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled , QIcon::Off, dfmbase::Global::ViewMode::kListMode,
                                                           isThumnailIconIndex(index) });
    }
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

bool ListItemPaintProxy::supportContentPreview() const
{
    return true;
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
