// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "viewgeometryhelper.h"
#include "views/fileview.h"
#include "views/private/fileview_p.h"
#include "models/fileviewmodel.h"
#include "views/baseitemdelegate.h"

#include <dfm-base/base/application/settings.h>
#include <dfm-base/dfm_global_defines.h>

#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QStyleOptionViewItem>

DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

ViewGeometryHelper::ViewGeometryHelper(FileView *parent)
    : QObject(parent),
      view(parent)
{
}

ViewGeometryHelper::RangeIndexList ViewGeometryHelper::visibleIndexes(const QRect &rect) const
{
    RangeIndexList list;

    QSize itemSize = view->itemSizeHint();

    int count = view->count();
    int spacing = view->spacing();
    int itemHeight = itemSize.height() + spacing * 2;

    if (view->isListViewMode() || view->isTreeViewMode()) {
        int firstIndex = (rect.top() + spacing) / itemHeight;
        int lastIndex = (rect.bottom() - spacing) / itemHeight;

        if (firstIndex >= count)
            return list;

        list << RangeIndex(qMax(firstIndex, 0), qMin(lastIndex, count - 1));
    } else if (view->isIconViewMode()) {

        // 分组绘制时计算区域内的list
        if (view->isGroupHeader(view->model()->index(0, 0, view->rootIndex()))) {
            list << calcGroupRectContiansIndexes(rect);
            return list;
        }

        int columnCount = view->d->calcColumnCount(rect.width(), itemSize.width());

        list << calcRectContiansIndexes(columnCount, rect);
    }

    return list;
}

ViewGeometryHelper::RangeIndexList ViewGeometryHelper::rectContainsIndexes(const QRect &rect) const
{
    RangeIndexList list;

    int count = view->count();
    if (count == 0)
        return list;

    if (view->isListViewMode() || view->isTreeViewMode()) {
        // For variable-height items (grouping enabled), use indexAtForSelection
        if (view->isGroupedView()) {
            // Convert rect to viewport coordinates for comparison
            QRect viewportRect = rect;
            viewportRect.translate(-view->horizontalOffset(), -view->verticalOffset());

            // Use indexAtForSelection to get items at corners (doesn't skip spacing areas)
            QModelIndex firstIndex = view->indexAtForSelection(viewportRect.topLeft());
            QModelIndex lastIndex = view->indexAtForSelection(viewportRect.bottomRight());

            // Handle invalid indices by clamping to valid range
            if (!firstIndex.isValid()) {
                // Check if above viewport
                if (viewportRect.top() < 0) {
                    firstIndex = view->model()->index(0, 0, view->rootIndex());
                } else {
                    return list;   // Empty selection
                }
            }

            if (!lastIndex.isValid()) {
                // Check if below viewport content
                if (viewportRect.bottom() >= 0) {
                    lastIndex = view->model()->index(count - 1, 0, view->rootIndex());
                } else {
                    return list;   // Empty selection
                }
            }

            if (firstIndex.isValid() && lastIndex.isValid()) {
                // Normalize rows to handle reversed selection (dragging upwards)
                int startRow = qMin(firstIndex.row(), lastIndex.row());
                int endRow = qMax(firstIndex.row(), lastIndex.row());
                list << RangeIndex(startRow, endRow);
            }
        } else {
            // Uniform height items (no grouping): use optimized calculation
            QSize itemSize = view->itemSizeHint();
            int spacing = view->spacing();
            int itemHeight = itemSize.height() + spacing * 2;

            int firstIndex = (rect.top() + spacing) / itemHeight;
            int lastIndex = (rect.bottom() - spacing) / itemHeight;

            if (firstIndex >= count)
                return list;

            list << RangeIndex(qMax(firstIndex, 0), qMin(lastIndex, count - 1));
        }
    } else if (view->isIconViewMode()) {
        QSize itemSize = view->itemSizeHint();
        int spacing = view->spacing();
        int itemWidth = itemSize.width() + spacing * 2;
        int columnCount = view->d->iconModeColumnCount(itemWidth);
        list << calcRectContiansIndexes(columnCount, rect);
    }

    return list;
}

ViewGeometryHelper::RangeIndexList ViewGeometryHelper::calcRectContiansIndexes(int columnCount, const QRect &rect) const
{
    RangeIndexList list {};

    QSize itemSize = view->itemSizeHint();
    QSize aIconSize = view->iconSize();

    int count = view->count();
    int spacing = view->spacing();
    int itemWidth = itemSize.width() + spacing * 2;
    int itemHeight = itemSize.height() + spacing * 2;
    QRect validRect = rect.marginsRemoved(QMargins(spacing, spacing, spacing, spacing));

    if (columnCount <= 0)
        return list;

    int beginRowIndex = validRect.top() / itemHeight;
    int endRowIndex = validRect.bottom() / itemHeight;
    int beginColumnIndex = validRect.left() / itemWidth;
    int endColumnIndex = validRect.right() / itemWidth;

    if (validRect.top() % itemHeight > aIconSize.height())
        ++beginRowIndex;

    int iconMargin = (itemWidth - aIconSize.width()) / 2;

    if (validRect.left() % itemWidth > itemWidth - iconMargin)
        ++beginColumnIndex;

    if (validRect.right() % itemWidth < iconMargin)
        --endColumnIndex;

    beginRowIndex = qMax(beginRowIndex, 0);
    beginColumnIndex = qMax(beginColumnIndex, 0);
    endRowIndex = qMin(endRowIndex, count / columnCount);
    endColumnIndex = qMin(endColumnIndex, columnCount - 1);

    if (beginRowIndex > endRowIndex || beginColumnIndex > endColumnIndex)
        return list;

    int beginIndex = beginRowIndex * columnCount;

    if (endColumnIndex - beginColumnIndex + 1 == columnCount) {
        list << RangeIndex(qMax(beginIndex, 0), qMin((endRowIndex + 1) * columnCount - 1, count - 1));

        return list;
    }

    for (int i = beginRowIndex; i <= endRowIndex; ++i) {
        if (beginIndex + beginColumnIndex >= count)
            break;

        list << RangeIndex(qMax(beginIndex + beginColumnIndex, 0),
                           qMin(beginIndex + endColumnIndex, count - 1));

        beginIndex += columnCount;
    }

    return list;
}

QRect ViewGeometryHelper::calcVisualRect(int widgetWidth, int index) const
{
    int iconViewSpacing = view->spacing();
    int iconHorizontalMargin = view->spacing();
#ifdef DTKWIDGET_CLASS_DSizeMode
    iconHorizontalMargin = DSizeModeHelper::element(kCompactIconHorizontalMargin, view->spacing());
#endif
    QSize itemSize = view->itemSizeHint();

    // 计算列数
    int columnCount = view->d->calcColumnCount(widgetWidth, itemSize.width());
    if (columnCount == 0)
        return QRect();

    int columnIndex = index % columnCount;
    int rowIndex = index / columnCount;

    int iconVerticalTopMargin = 0;
#ifdef DTKWIDGET_CLASS_DSizeMode
    iconVerticalTopMargin = DSizeModeHelper::element(kCompactIconVerticalTopMargin, view->spacing());
#endif

    QRect rect;
    // 计算顶部位置：上边距(VerticalTopMargin) + 行索引 * (项目高度 + 间距)
    rect.setTop(iconVerticalTopMargin + rowIndex * (itemSize.height() + 2 * iconViewSpacing));

    // 计算左侧位置：左边距(kIconHorizontalMargin) + 列索引 * (项目宽度 + 间距)
    rect.setLeft(iconHorizontalMargin + columnIndex * (itemSize.width() + 2 * iconViewSpacing));

    rect.setSize(itemSize);

    // 计算水平居中偏移，仅当行数大于1时才应用
    // In group mode, don't apply horizontal centering to keep group headers left-aligned
    if (!view->isGroupedView()) {
        int totalItems = view->model()->rowCount();
        int rowCount = (totalItems + columnCount - 1) / columnCount;   // 向上取整
        if (rowCount > 1) {
            // 计算可用宽度（减去左右边距）
            int availableWidth = widgetWidth - 2 * iconHorizontalMargin;
            int totalItemsWidth = columnCount * itemSize.width() + (columnCount - 1) * 2 * iconViewSpacing;
            int horizontalOffset = (availableWidth - totalItemsWidth) / 2;
            rect.moveLeft(rect.left() + horizontalOffset);
        }
    }

    rect.moveTop(rect.top() - view->verticalOffset());

    return rect;
}

bool ViewGeometryHelper::indexInRect(const QRect &actualRect, const QModelIndex &index) const
{
    auto paintRect = view->visualRect(index);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto opt = view->viewOptions();
#else
    QStyleOptionViewItem opt;
    view->initViewItemOption(&opt);
#endif
    opt.rect = paintRect;
    auto rectList = view->itemDelegate()->itemGeomertys(opt, index);
    for (const auto &rect : rectList) {
        if (!(actualRect.left() > rect.right()
              || actualRect.top() > rect.bottom()
              || rect.left() > actualRect.right()
              || rect.top() > actualRect.bottom()))
            return true;
    }

    return false;
}

ViewGeometryHelper::RangeIndexList ViewGeometryHelper::calcGroupRectContiansIndexes(const QRect &rect) const
{
    // 计算当前区域内的index，目前采用的是遍历的方式，这个有效率问题
    // 后面优化，通过每行绘制个数，每个绘制大小，每个分组的个数，先计算出来大致的一个index的范围，再判断这个范围内的index
    RangeIndexList list {};
    int begin = -1, end = -1;
    for (int i = 0; i < view->model()->rowCount(); ++i) {
        auto indexRect = view->visualRect(view->model()->index(i, 0, view->rootIndex()));
        if (rect.intersects(indexRect)) {
            if (begin < 0)
                begin = i;
            end = i;
        } else if (indexRect.top() >= rect.bottom() && indexRect.top() >= rect.top()) {
            break;
        }
    }

    list << RangeIndex(begin, end);
    return list;
}
