// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "listitempaintproxy.h"
#include "utils/itemdelegatehelper.h"

using namespace dfmplugin_workspace;

ListItemPaintProxy::ListItemPaintProxy(QObject *parent)
    : AbstractItemPaintProxy(parent)
{
}

void ListItemPaintProxy::drawIcon(QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(index)

    bool isEnabled = option.state & QStyle::State_Enabled;
    ItemDelegateHelper::paintIcon(painter, option.icon, *rect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled);
}

