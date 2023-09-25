// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treeitempaintproxy.h"
#include "fileview.h"
#include "utils/itemdelegatehelper.h"

#include "dfm-base/dfm_global_defines.h"

#include <QStyle>

DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

TreeItemPaintProxy::TreeItemPaintProxy(QObject *parent)
    : AbstractItemPaintProxy(parent)
{

}

void TreeItemPaintProxy::drawIcon(QPainter *painter, QRectF *rect, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(index)

    int depth = index.data(kItemTreeViewDepthRole).toInt();

    int iconIntent = (kTreeItemIndent * depth) + kTreeExpandArrowWidth;

    bool isEnabled = option.state & QStyle::State_Enabled;
    rect->moveLeft(rect->left() + iconIntent);
    ItemDelegateHelper::paintIcon(painter, option.icon, *rect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled);

    if (index.data(kItemTreeViewCanExpandRole).toBool())
        drawExpandArrow(painter, *rect, option, index);
}

QRectF TreeItemPaintProxy::iconRect(const QModelIndex &index, const QRect &itemRect)
{
    QSize iconSize = view()->iconSize();

    return QRectF();
}

QRect TreeItemPaintProxy::drawExpandArrow(QPainter *painter, const QRectF &rect, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QStyleOptionViewItem opt = option;
    QRect arrowRect = opt.rect;

    arrowRect.setRight(static_cast<int>(rect.left()) - 5);
    arrowRect.setLeft(arrowRect.right() - kTreeExpandArrowWidth);
    arrowRect.setHeight(kTreeExpandArrowHeight);
    arrowRect.moveCenter(QPoint(arrowRect.left() + (kTreeExpandArrowWidth / 2), static_cast<int>(rect.center().y())));
    opt.rect = arrowRect;

    if (index.data(kItemTreeViewExpandabledRole).toBool()) {
        style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, painter, nullptr);
    } else {
        style->drawPrimitive(QStyle::PE_IndicatorArrowRight, &opt, painter, nullptr);
    }

    return arrowRect;
}

FileView *TreeItemPaintProxy::view()
{
    return qobject_cast<FileView *>(parent());
}
