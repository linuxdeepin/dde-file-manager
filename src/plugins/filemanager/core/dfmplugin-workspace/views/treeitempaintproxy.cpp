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

    int nameColumnWidth = view()->getColumnWidth(0);
    firstColumnRightBoundary = option.rect.x() + nameColumnWidth - 1 - view()->viewportMargins().left();

    if (rect->right() <= firstColumnRightBoundary) {
        bool isEnabled = option.state & QStyle::State_Enabled;
        ItemDelegateHelper::paintIcon(painter, option.icon, { *rect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled });
    }

    if (index.data(kItemTreeViewCanExpandRole).toBool())
        drawExpandArrow(painter, *rect, option, index);
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

QList<QRect> TreeItemPaintProxy::allPaintRect(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QList<QRect> rects {};
    QRect itemRect = option.rect;
    QRectF icon = iconRect(index, itemRect);

    rects.append(icon.toRect());
    rects.append(arrowRect(icon).toRect().marginsAdded(QMargins(5, 5, 5, 5)));

    return rects;
}

int TreeItemPaintProxy::iconRectIndex()
{
    return 1;
}

void TreeItemPaintProxy::drawExpandArrow(QPainter *painter, const QRectF &rect, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QStyleOptionViewItem opt = option;
    QRectF arrowRect = this->arrowRect(rect);

    // the real arrow size to paint smaller than the given rect
    opt.rect = arrowRect.toRect().marginsRemoved(QMargins(5, 5, 5, 5));

    if (opt.rect.right() > firstColumnRightBoundary)
        return;

    painter->save();
    bool isSelected = (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;
    if (isSelected) {
        painter->setPen(opt.palette.color(QPalette::Active, QPalette::HighlightedText));
    } else {
        painter->setPen(opt.palette.color(QPalette::Active, QPalette::Text));
    }

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    if (index.data(kItemTreeViewExpandedRole).toBool()) {
        style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &opt, painter, nullptr);
    } else {
        style->drawPrimitive(QStyle::PE_IndicatorArrowRight, &opt, painter, nullptr);
    }

    painter->restore();
}

QRectF TreeItemPaintProxy::iconRect(const QModelIndex &index, const QRect &itemRect)
{
    QRectF iconRect = itemRect;
    QSize iconSize = view()->iconSize();
    iconRect.setSize(iconSize);

    int depth = index.data(kItemTreeViewDepthRole).toInt();
    int iconIntent = (kTreeItemIndent * depth) + kTreeExpandArrowWidth - kTreeArrowAndIconDistance;

    iconRect.moveLeft(iconRect.left() + kListModeLeftMargin + kListModeLeftPadding + iconIntent);
    iconRect.moveTop(iconRect.top() + ((itemRect.bottom() - iconRect.bottom()) / 2));

    return iconRect;
}

QRectF TreeItemPaintProxy::arrowRect(const QRectF &iconRect)
{
    QRectF arrowRect = iconRect;

    arrowRect.setSize(QSizeF(kTreeExpandArrowWidth, kTreeExpandArrowHeight));
    arrowRect.moveTop(iconRect.top() + (iconRect.bottom() - arrowRect.bottom()) / 2);
    arrowRect.moveCenter(QPointF(iconRect.left() - kTreeArrowAndIconDistance, arrowRect.center().y()));

    return arrowRect;
}

FileView *TreeItemPaintProxy::view()
{
    return qobject_cast<FileView *>(parent());
}
