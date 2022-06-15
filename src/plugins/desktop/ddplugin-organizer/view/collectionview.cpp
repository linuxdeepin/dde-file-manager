/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "collectionview_p.h"
#include "dfm-base/utils/windowutils.h"
#include "models/fileproxymodel.h"
#include "delegate/collectionitemdelegate.h"

#include <QScrollBar>
#include <QUrl>
#include <QDebug>
#include <QPainter>

DDP_ORGANIZER_USE_NAMESPACE

CollectionViewPrivate::CollectionViewPrivate(CollectionView *qq)
    : q(qq)
{

}

void CollectionViewPrivate::updateViewSizeData(const QSize &viewSize, const QMargins &viewMargins, const QSize &itemSize)
{
    // 固定行高与间隔边距,无行数限制
    // top margin is for icon top spacing
    // todo:define ICON_TOP_SPACE_DESKTOP 2
    static const QMargins minMargin(0, 2, 0, 0);
    int minCellWidth = itemSize.width() + minMargin.left() + minMargin.right();
    updateColumnCount(viewSize.width(), minCellWidth);

    int minCellHeight = itemSize.height() + minMargin.top() + minMargin.bottom();
    updateRowCount(viewSize.height(), minCellHeight);

    updateCellMargins(itemSize, QSize(cellWidth, cellHeight));

    updateViewMargins(viewSize, viewMargins);
}

int CollectionViewPrivate::verticalScrollToValue(const QModelIndex &index, const QRect &rect, QAbstractItemView::ScrollHint hint) const
{
    Q_UNUSED(index)

    const QRect &&area = q->viewport()->rect();
    const bool above = (QAbstractItemView::EnsureVisible == hint && rect.top() < area.top());
    const bool below = (QAbstractItemView::EnsureVisible == hint && rect.bottom() > area.bottom());

    int verticalValue = q->verticalScrollBar()->value();
    QRect adjusted = rect.adjusted(-space, -space, space, space);
    if (QAbstractItemView::PositionAtTop == hint || above) {
        verticalValue += adjusted.top();
    } else if (QAbstractItemView::PositionAtBottom || below) {
        verticalValue += qMin(adjusted.top(), adjusted.bottom() - area.height() + 1);
    } else if (QAbstractItemView::PositionAtCenter == hint) {
        verticalValue += adjusted.top() - ((area.height() - adjusted.height()) / 2);
    }

    return verticalValue;
}

QItemSelection CollectionViewPrivate::selection(const QRect &rect) const
{
    QItemSelection selection;
    const QRect actualRect(qMin(rect.left(), rect.right())
                           , qMin(rect.top(), rect.bottom()) + q->verticalOffset()
                           , abs(rect.width())
                           , abs(rect.height()));
    const QPoint offset(-q->horizontalOffset(), 0);
    const QPoint iconOffset(10, 10);    // todo(wangcl):define

    for (auto url : urls) {
        auto index = q->model()->index(url);
        const QRect &&itemRect = q->visualRect(index);
        QRect realItemRect(itemRect.topLeft() + offset + iconOffset, itemRect.bottomRight() + offset - iconOffset);

        // least 3 pixels
        static const int diff = 3;
        if (actualRect.left() > realItemRect.right() - diff
                || actualRect.top() > realItemRect.bottom() - diff
                || actualRect.right() < realItemRect.left() + diff
                || actualRect.bottom() < realItemRect.top() + diff)
            continue;

        if (!selection.contains(index)) {
            selection.push_back(QItemSelectionRange(index));
        }
    }

    return selection;
}

QPoint CollectionViewPrivate::pointToPos(const QPoint &point) const
{
    int column = (point.x() - viewMargins.left()) / cellWidth;
    int row = (point.y() - viewMargins.top()) / cellHeight;
    return QPoint(row, column);
}

QPoint CollectionViewPrivate::posToPoint(const QPoint &pos) const
{
    int pointX = pos.y() * cellWidth + viewMargins.left();
    int pointY = pos.x() * cellHeight + viewMargins.top();
    return QPoint(pointX, pointY);
}

int CollectionViewPrivate::posToNode(const QPoint &pos) const
{
    return pos.x() * columnCount + pos.y();
}

QPoint CollectionViewPrivate::nodeToPos(const int node) const
{
    return QPoint(node / columnCount, node % columnCount);
}

void CollectionViewPrivate::updateRowCount(const int &viewHeight, const int &minCellHeight)
{
    rowCount = viewHeight / minCellHeight;
    if (Q_UNLIKELY(rowCount < 1)) {
        qWarning() << "Row count is 0!Fix it to 1,and set cell height to:" << minCellHeight;
        cellHeight = minCellHeight;
        rowCount = 1;
    } else {
        cellHeight = viewHeight / rowCount;
    }

    if (Q_UNLIKELY(cellHeight < 1)) {
        qWarning() << "Cell height is:" << cellHeight << "!Fix it to 1";
        cellHeight = 1;
    }
}

void CollectionViewPrivate::updateColumnCount(const int &viewWidth, const int &minCellWidth)
{
    columnCount = viewWidth / minCellWidth;
    if (Q_UNLIKELY(columnCount < 1)) {
        qWarning() << "Column count is 0!Fix it to 1,and set cell width to:" << viewWidth;
        cellWidth = viewWidth;
        columnCount = 1;
    } else {
        cellWidth = viewWidth / columnCount;
    }

    if (Q_UNLIKELY(cellWidth < 1)) {
        qWarning() << "Cell width is:" << cellWidth << "!Fix it to 1";
        cellWidth = 1;
    }
}

void CollectionViewPrivate::updateCellMargins(const QSize &itemSize, const QSize &cellSize)
{
    // todo:UI不合理，待确认
    const int horizontalMargin = cellSize.width() - itemSize.width();
    const int verticalMargin = cellSize.height() - itemSize.height();
    const int leftMargin = horizontalMargin / 2;
    const int rightMargin = horizontalMargin - leftMargin;
    const int topMargin = verticalMargin / 2;
    const int bottomMargin = verticalMargin - topMargin;

    cellMargins = QMargins(leftMargin, topMargin, rightMargin, bottomMargin);
}

void CollectionViewPrivate::updateViewMargins(const QSize &viewSize, const QMargins &oldMargins)
{
    const int horizontalMargin = (viewSize.width() - cellWidth * columnCount);
    const int leftMargin = horizontalMargin / 2;
    const int rightMargin = horizontalMargin - leftMargin;
    const int topMargin = 0;
    const int bottomMargin = 0;

    viewMargins = oldMargins + QMargins(leftMargin, topMargin, rightMargin, bottomMargin);
}

CollectionView::CollectionView(QWidget *parent)
    : QAbstractItemView(parent)
    , d(new CollectionViewPrivate(this))
{
    initUI();
}

QList<QUrl> CollectionView::urls() const
{
    return d->urls;
}

void CollectionView::setUrls(const QList<QUrl> &urls)
{
    d->urls.clear();
    for (auto url : urls) {
        if (url.fileName().startsWith("."))
            continue;
        d->urls.append(url);
    }

    // todo:在holder中判断,只有自动集合才隐藏空集合
    if (d->urls.isEmpty())
        hide();
    else
        show();

    updateGeometries();
    update();
}

QMargins CollectionView::cellMargins() const
{
    return d->cellMargins;
}

FileProxyModel *CollectionView::model() const
{
    return qobject_cast<FileProxyModel *>(QAbstractItemView::model());
}

CollectionItemDelegate *CollectionView::itemDelegate() const
{
    return qobject_cast<CollectionItemDelegate *>(QAbstractItemView::itemDelegate());
}

QRect CollectionView::visualRect(const QModelIndex &index) const
{
    if (!index.isValid())
        return QRect();

    QUrl url = model()->fileUrl(index);
    if (!d->urls.contains(url))
        return QRect();

    int node = d->urls.indexOf(url);
    const QPoint &&pos = d->nodeToPos(node);
    const QPoint &&point = d->posToPoint(pos);

    QRect rect(point.x(), point.y(), d->cellWidth, d->cellHeight);
    rect.moveLeft(rect.left() - horizontalOffset());
    rect.moveTop(rect.top() - verticalOffset());

    return rect;
}

void CollectionView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    const QRect &&rect = visualRect(index);
    if (!rect.isValid())
        return;

    if (EnsureVisible == hint && viewport()->rect().contains(rect)) {
        viewport()->update(rect);
        return;
    }

    // vertical
    verticalScrollBar()->setValue(d->verticalScrollToValue(index, rect, hint));

    // disable horizontal
}

QModelIndex CollectionView::indexAt(const QPoint &point) const
{
    auto checkRect = [](const QList<QRect> &listRect, const QPoint &point) -> bool {
        // icon rect
        if (listRect.size() > 0 && listRect.at(0).contains(point))
            return true;

        if (listRect.size() > 1) {
            QRect identify = listRect.at(1);
            if (identify.contains(point))
                return true;
        }
        return false;
    };

    QModelIndex rowIndex = currentIndex();
    // first check the editing item or the expended item.
    // the editing item and the expended item must be one item.
    if (rowIndex.isValid() && isPersistentEditorOpen(rowIndex)) {
        QList<QRect> identify;
        // editor area that the height is higher than visualRect.
        if (QWidget *editor = indexWidget(rowIndex))
            identify << editor->geometry();
        if (checkRect(identify, point)) {
            return rowIndex;
        }
    } else if (itemDelegate()->mayExpand(&rowIndex)) {   // second
        // get the expended rect.
        auto listRect = itemPaintGeomertys(rowIndex);
        if (checkRect(listRect, point)) {
            return rowIndex;
        }
    }

    // then check the item on the point.
    {
        int node = d->posToNode(d->pointToPos(point));
        if (node >= d->urls.count())
            return QModelIndex();
        rowIndex = model()->index(d->urls.at(node));
        if (!rowIndex.isValid())
            return rowIndex;

        auto listRect = itemPaintGeomertys(rowIndex);
        if (checkRect(listRect, point)) {
            return rowIndex;
        }
    }

    return QModelIndex();
}

QModelIndex CollectionView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    auto findAvailableRowBackward = [=](int row) {
        if (row >= 0)
            --row;
        return row;
    };

    auto findAvailableRowForward = [=](int row) {
        if (row < d->rowCount)
            ++row;
        return row;
    };

    QModelIndex current = currentIndex();
    if (!current.isValid()) {
        int row = findAvailableRowForward(0);   // select first
        if (-1 == row || d->urls.isEmpty())
            return QModelIndex();
        const QUrl &url = d->urls.first();
        return model()->index(url);
    }

    switch (cursorAction) {
    case MoveLeft : {
        // todo:通过current从model中获取url，再从gridManager中获取该url的上一个url，再从model中获取对应的index
        return QModelIndex();
    }
    case MoveRight : {
        // todo:通过current从model中获取url，再从gridManager中获取该url的下一个url，再从model中获取对应的index
        return QModelIndex();
    }
    case MoveUp :
    case MovePrevious : {
        // todo:通过current从model中获取url,再从gridManager中获取该url的网格位置,获取其上一行的位置处的url,再从model获取该url对应的index
        return QModelIndex();
    }
    case MoveDown :
    case MoveNext : {
        // todo:通过current从model中获取url,再从gridManager中获取该url的网格位置,获取其下一行的位置处的url,再从model获取该url对应的index
        return QModelIndex();
    }
    case MoveHome : {
        // todo:返回第一个
        return QModelIndex();
    }
    case MoveEnd : {
        // todo:返回最后一个
        return QModelIndex();
    }
    case MovePageUp : {
        // todo:返回上一页的第一个
        // todo:shift?范围选择
        return QModelIndex();
    }
    case MovePageDown : {
        // todo:返回下一页的最后一个
        // todo:shift?范围选择
        return QModelIndex();
    }
    }
}

int CollectionView::horizontalOffset() const
{
    if (Q_UNLIKELY(isRightToLeft()))
        return horizontalScrollBar()->maximum() - horizontalScrollBar()->value();

    return horizontalScrollBar()->value();
}

int CollectionView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

bool CollectionView::isIndexHidden(const QModelIndex &index) const
{
    Q_UNUSED(index)

    // disable hidden index
    return false;
}

void CollectionView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    if (!selectionModel())
        return;

    QItemSelection selection;

    if (rect.width() == 1 && rect.height() == 1) {
        QModelIndex currentIndex = indexAt(rect.center());
        if (currentIndex.isValid())
            selection.select(currentIndex, currentIndex);
    } else {
        auto realRect = rect.translated(horizontalOffset(), verticalOffset());
        selection = d->selection(realRect);
    }

    selectionModel()->select(selection, command);
}

QRegion CollectionView::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion region;
    auto selectedList = selection.indexes();
    for (auto &index : selectedList)
        region = region.united(QRegion(visualRect(index)));

    return region;
}

void CollectionView::paintEvent(QPaintEvent *event)
{
    if (Q_UNLIKELY(!itemDelegate()))
        return;

    auto option = viewOptions();
    QPainter painter(viewport());

    auto repaintRect = viewport()->geometry().translated(horizontalOffset(), verticalOffset());
    auto topLeft = repaintRect.topLeft();
    auto bottomRight = repaintRect.bottomRight();
    auto startPoint = d->pointToPos(topLeft);
    auto startNode = d->posToNode(startPoint);
    auto endPoint = d->pointToPos(bottomRight);
    auto endNode = d->posToNode(endPoint);

    if (d->showGrid) {
        painter.save();
        for (auto node = 0 ; node <= endNode - startNode; ++node) {
            auto pos = d->nodeToPos(node);
            auto point = d->posToPoint(pos);
            auto rect = QRect(point.x(), point.y(), d->cellWidth, d->cellHeight);

            auto rowMode = pos.x() % 2;
            auto colMode = pos.y() % 2;
            auto color = (colMode == rowMode) ? QColor(0, 0, 255, 32) : QColor(255, 0, 0, 32);
            painter.setPen(Qt::darkGray);
            painter.drawRect(rect);
            painter.fillRect(rect, color);

            auto itemSize = rect.marginsRemoved(d->cellMargins);
            painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
            painter.drawRect(itemSize);

            painter.setPen(QPen(Qt::red, 2));
            painter.drawText(rect, QString("%1-%2").arg(d->nodeToPos(node + startNode).x()).arg(d->nodeToPos(node + startNode).y()));
        }
        painter.restore();
    }

    for (auto node = startNode ; node <= endNode ; ++node) {
        if (node >= d->urls.count())
            break;

        auto url = d->urls.at(node);
        auto index = model()->index(url);
        option.rect = visualRect(index).marginsRemoved(d->cellMargins);
        painter.save();
        itemDelegate()->paint(&painter, option, index);
        painter.restore();
    }
}

void CollectionView::wheelEvent(QWheelEvent *event)
{
    auto aaa = verticalScrollBar()->sliderPosition();
    auto bbb = event->angleDelta().y();
    auto ccc = aaa - bbb;
    verticalScrollBar()->setSliderPosition( ccc );
}

void CollectionView::mouseMoveEvent(QMouseEvent *event)
{
    return QAbstractItemView::mouseMoveEvent(event);
}

void CollectionView::resizeEvent(QResizeEvent *event)
{
    QAbstractItemView::resizeEvent(event);

    updateRegionView();
}

void CollectionView::updateGeometries()
{
    int dataRow = d->urls.count() / d->columnCount;
    if (0 != d->urls.count() % d->columnCount)
        dataRow += 1;

    int height = dataRow * d->cellHeight + d->viewMargins.top() + d->viewMargins.bottom() - viewport()->height();

    verticalScrollBar()->setRange(0, height);
    qDebug() << "update vertical scrollbar range to:" << verticalScrollBar()->maximum();

    return QAbstractItemView::updateGeometries();
}

void CollectionView::initUI()
{
    setRootIndex(model()->rootIndex());
    setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAutoFillBackground(false);
    setFrameShape(QFrame::NoFrame);

    auto delegate = new CollectionItemDelegate(this);
    setItemDelegate(delegate);

    // todo:disble selection???

//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void CollectionView::updateRegionView()
{
    itemDelegate()->updateItemSizeHint();
    auto itemSize = itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex());

    d->updateViewSizeData(this->geometry().size(), QMargins(0, 0, 0, 0), itemSize);
}

QList<QRect> CollectionView::itemPaintGeomertys(const QModelIndex &index) const
{
    if (Q_UNLIKELY(!index.isValid()))
        return {};

    QStyleOptionViewItem option = viewOptions();
    option.rect = itemRect(index);
    return itemDelegate()->paintGeomertys(option, index);
}

QRect CollectionView::itemRect(const QModelIndex &index) const
{
    return visualRect(index).marginsRemoved(d->cellMargins);
}

