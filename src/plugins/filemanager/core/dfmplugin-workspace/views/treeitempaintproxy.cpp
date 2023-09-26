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
    *rect = iconRect(index, rect->toRect());

    bool isEnabled = option.state & QStyle::State_Enabled;
    ItemDelegateHelper::paintIcon(painter, option.icon, *rect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled);

    if (index.data(kItemTreeViewCanExpandRole).toBool()) {
        drawExpandArrow(painter, *rect, option, index);

        // test block
        {
//            QRect itemRect = view()->visualRect(index);
//            QRectF rectI = iconRect(index, itemRect);
//            QRectF rectA = arrowRect(rectI);
//            rectA = rectA.marginsAdded(QMarginsF(5, 5,5,5));
//            painter->save();
//            painter->setPen(Qt::red);
//            painter->setBrush(Qt::red);
//            painter->drawRect(rectA);
//            painter->restore();
        }
    }
}

QRectF TreeItemPaintProxy::rectByType(RectOfItemType type, const QModelIndex &index)
{
    QRect itemRect = view()->visualRect(index);
    switch (type) {
    case RectOfItemType::kItemIconRect:
        return iconRect(index, itemRect);
    case RectOfItemType::kItemTreeArrowRect:
        QRectF iconRect = this->iconRect(index, itemRect);
        return arrowRect(iconRect);
    }

    return QRectF();
}

void TreeItemPaintProxy::drawExpandArrow(QPainter *painter, const QRectF &rect, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QStyleOptionViewItem opt = option;
    QRectF arrowRect = this->arrowRect(rect);

    opt.rect = arrowRect.toRect();

    if (index.data(kItemTreeViewExpandabledRole).toBool()) {
        style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, painter, nullptr);
    } else {
        style->drawPrimitive(QStyle::PE_IndicatorArrowRight, &opt, painter, nullptr);
    }
}

QRectF TreeItemPaintProxy::iconRect(const QModelIndex &index, const QRect &itemRect)
{
    QRectF iconRect = itemRect;
    QSize iconSize = view()->iconSize();
    iconRect.setSize(iconSize);

    int depth = index.data(kItemTreeViewDepthRole).toInt();
    int iconIntent = (kTreeItemIndent * depth) + kTreeExpandArrowWidth;

    iconRect.moveLeft(iconRect.left() + kListModeLeftMargin + + kListModeLeftPadding + iconIntent);
    iconRect.moveTop(iconRect.top() + ((itemRect.bottom() - iconRect.bottom()) / 2));

    return iconRect;
}

QRectF TreeItemPaintProxy::arrowRect(const QRectF &iconRect)
{
    QRectF arrowRect = iconRect;

    arrowRect.moveLeft(iconRect.left() - kTreeExpandArrowWidth - 5);
    arrowRect.setSize(QSizeF(kTreeExpandArrowWidth, kTreeExpandArrowHeight));
    arrowRect.moveTop(iconRect.top() + (iconRect.bottom() - arrowRect.bottom()) / 2);

    return arrowRect;
}

FileView *TreeItemPaintProxy::view()
{
    return qobject_cast<FileView *>(parent());
}
