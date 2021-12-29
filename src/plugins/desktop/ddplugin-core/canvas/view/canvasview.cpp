/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
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

#include "view/canvasview_p.h"
#include "operator/boxselecter.h"
#include "operator/viewpainter.h"
#include "delegate/canvasitemdelegate.h"
#include "grid/canvasgrid.h"
#include "displayconfig.h"

#include "base/schemefactory.h"

#include <QGSettings>

#include <QPainter>
#include <QDebug>
#include <QScrollBar>
#include <QPaintEvent>
#include <QApplication>


DSB_D_USE_NAMESPACE

CanvasView::CanvasView(QWidget *parent)
    : QAbstractItemView(parent)
    , d(new CanvasViewPrivate(this))
{

}

QRect CanvasView::visualRect(const QModelIndex &index) const
{
    return d->visualRect(model()->url(index).toString());
}

void CanvasView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint) {
    Q_UNUSED(index)
            Q_UNUSED(hint)
}

QModelIndex CanvasView::indexAt(const QPoint &point) const
{
    QString item = d->visualItem(d->gridAt(point));
    QModelIndex rowIndex = model()->index(item, 0);
    auto listRect = itemPaintGeomertys(rowIndex);

    // icon rect
    if (listRect.size() > 0 && listRect.at(0).contains(point))
        return rowIndex;

    if (listRect.size() > 2) {
        QRect label = listRect.at(1);
        QRect text = listRect.at(2);

        //identification area is text rect spread upward to label.
        text.setTop(label.top());
        if (text.contains(point))
            return rowIndex;
    }

    return QModelIndex();
}

QModelIndex CanvasView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(cursorAction)
    Q_UNUSED(modifiers)
    return QModelIndex();
}

int CanvasView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

int CanvasView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

bool CanvasView::isIndexHidden(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return false;
}

void CanvasView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    //! do not enable QAbstractItemView using this to select.
    //! it will disturb selections of CanvasView
    //qWarning() << __FUNCTION__ << "do not using this" << rect.normalized();
    return;

    QItemSelection selection;
    BoxSelIns->selection(this, rect.normalized(), &selection);
    selectionModel()->select(selection, command);
}

QRegion CanvasView::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion region;
    auto selectedList = selection.indexes();
    for (auto &index : selectedList)
        region = region.united(QRegion(visualRect(index)));

    return region;
}

QList<QRect> CanvasView::itemPaintGeomertys(const QModelIndex &index) const
{
    QStyleOptionViewItem option = viewOptions();
    option.rect = itemRect(index);
    return itemDelegate()->paintGeomertys(option, index);
}

void CanvasView::paintEvent(QPaintEvent *event)
{
    ViewPainter painter(d.get());
    painter.setRenderHints(QPainter::HighQualityAntialiasing);

    // debug网格信息展示
    painter.drawGirdInfos();

    // todo:让位
    painter.drawDodge();

    // 桌面文件绘制
    auto option = viewOptions();

    painter.paintFiles(option, event);

    // 绘制选中区域
    painter.drawSelectRect();

    // todo: 拖动绘制
    painter.drawDragMove(option);
}

void CanvasView::setScreenNum(const int screenNum)
{
    d->screenNum = screenNum;
}

int CanvasView::screenNum() const
{
    return d->screenNum;
}

CanvasItemDelegate *CanvasView::itemDelegate() const
{
    return qobject_cast<CanvasItemDelegate *>(QAbstractItemView::itemDelegate());
}

CanvasModel *CanvasView::model() const
{
    return qobject_cast<CanvasModel *>(QAbstractItemView::model());
}

CanvasSelectionModel *CanvasView::selectionModel() const
{
    return qobject_cast<CanvasSelectionModel *>(QAbstractItemView::selectionModel());
}

void CanvasView::setGeometry(const QRect &rect)
{
    if (rect.size().width() < 1 || rect.size().height() < 1) {
        return;
    } else {
        QAbstractItemView::setGeometry(rect);
        updateGrid();

        if (d->waterMask)
            d->waterMask->refresh();
    }
}

void CanvasView::updateGrid()
{
    // todo:
    itemDelegate()->updateItemSizeHint();
    auto itemSize = itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex());

    // add view margin. present is none.
    const QMargins geometryMargins = QMargins(0, 0, 0, 0);
    d->updateGridSize(geometry().size(), geometryMargins, itemSize);

    GridIns->updateSize(d->screenNum, QSize(d->canvasInfo.columnCount, d->canvasInfo.rowCount));

    //todo update expend item if needed.
    //auto expandedWidget = reinterpret_cast<QWidget *>(itemDelegate()->expandedIndexWidget());

    update();
}

bool CanvasView::isTransparent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    // TODO(Lee): cut and staging files are transparent

    return false;
}

QList<QIcon> CanvasView::additionalIcon(const QModelIndex &index) const
{
    Q_UNUSED(index)
    QList<QIcon> list;
    // TODO(LIQIANG)： get additional Icon

    return list;
}

QRect CanvasView::itemRect(const QModelIndex &index) const
{
    return d->itemRect(model()->url(index).toString());
}

void CanvasView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QAbstractItemView::mousePressEvent(event);
        return;
    }

    auto index = indexAt(event->pos());
    if (!index.isValid()) { //empty area
        BoxSelIns->beginSelect(event->globalPos(), true);
        setState(DragSelectingState);
    }

    {
        d->clickSelecter->click(index);
        QAbstractItemView::mousePressEvent(event);
    }
}

void CanvasView::mouseMoveEvent(QMouseEvent *event)
{
    QAbstractItemView::mouseMoveEvent(event);
}

void CanvasView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QAbstractItemView::mouseReleaseEvent(event);
        return;
    }

    auto releaseIndex = indexAt(event->pos());
    d->clickSelecter->release(releaseIndex);

    setState(NoState);
    QAbstractItemView::mouseReleaseEvent(event);
}

void CanvasView::initUI()
{
    setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAutoFillBackground(false);
    setFrameShape(QFrame::NoFrame);

    // using NoSelection to turn off selection of QAbstractItemView
    // and CanvasView will to do selection by itself.
    setSelectionMode(QAbstractItemView::NoSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);

    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    setDefaultDropAction(Qt::CopyAction);

    // init icon delegate
    auto delegate = new CanvasItemDelegate(this);
    setItemDelegate(delegate);
    delegate->setIconLevel(DispalyIns->iconLevel());

    // repaint when selecting with mouse move.
    connect(BoxSelIns, &BoxSelecter::changed, this, static_cast<void (CanvasView::*)()>(&CanvasView::update));

    // water mask
    if (d->isWaterMaskOn()) {
        Q_ASSERT(!d->waterMask);
        d->waterMask = new WaterMaskFrame("/usr/share/deepin/dde-desktop-watermask.json", this);
        d->waterMask->lower();
        d->waterMask->refresh();
    }
}

const QMargins CanvasViewPrivate::gridMiniMargin = QMargins(2, 2, 2, 2);

// dockReserveSize leads to lessen the column and row，and increase the width and height of grid by expanding grid margin.
// keep it for compatibility. Remove it if need reduce the grid margin
const QSize CanvasViewPrivate::dockReserveSize = QSize(80, 80);

CanvasViewPrivate::CanvasViewPrivate(CanvasView *qq)
    : QObject(qq)
    , q(qq)
{
#ifdef QT_DEBUG
    showGrid = true;
#endif
    clickSelecter = new ClickSelecter(qq);
}

CanvasViewPrivate::~CanvasViewPrivate()
{
    clickSelecter = nullptr;
}

void CanvasViewPrivate::updateGridSize(const QSize &viewSize, const QMargins &geometryMargins, const QSize &itemSize)
{
    // canvas size is view size minus geometry margins.
    QSize canvasSize(viewSize.width() - geometryMargins.left() - geometryMargins.right(),
                     viewSize.height() - geometryMargins.top() - geometryMargins.bottom());
    qInfo() << "view size" << viewSize << "canvas size" << canvasSize  << "view margin" << geometryMargins << "item size" << itemSize;

    if (canvasSize.width() < 1 || canvasSize.height() < 1) {
        qCritical() << "canvas size is invalid.";
        return;
    }

    // the minimum width of each grid.
    const int miniGridWidth = itemSize.width() + gridMiniMargin.left() + gridMiniMargin.right();

    // mins dockReserveSize is to keep column count same as the old.
    // it leads to fewer column count and widen the grid margin.
    int columnCount = (canvasSize.width() - dockReserveSize.width()) / miniGridWidth;
    int gridWidth = 1;
    if (Q_UNLIKELY(columnCount < 1)) {
        qCritical() << " column count is 0. set it to 1 and set grid width to " << canvasSize.width();
        gridWidth = canvasSize.width();
        columnCount = 1;
    } else {
        gridWidth = canvasSize.width() / columnCount;
    }

    if (Q_UNLIKELY(gridWidth < 1))
        gridWidth = 1;

    // the minimum height of each grid.
    const int miniGridHeight = itemSize.height() + gridMiniMargin.top() + gridMiniMargin.bottom();
    int gridHeight = 1;

    // mins dockReserveSize is to keep row count same as the old.
    // it leads to fewer row count and rise the grid margin.
    int rowCount = (canvasSize.height() - dockReserveSize.height())/ miniGridHeight;
    if (Q_UNLIKELY(rowCount < 1)) {
        qCritical() << "row count is 0. set it to 1 and set grid height to" << canvasSize.height();
        gridHeight = canvasSize.height();
        rowCount = 1;
    } else {
        gridHeight = canvasSize.height() / rowCount;
    }

    if (Q_UNLIKELY(gridHeight < 1))
        gridHeight = 1;

    // margin for each gird
    gridMargins = calcMargins(itemSize, QSize(gridWidth, gridHeight));

    // margins around the view，canvas gemotry is view gemotry minus viewMargins.
    viewMargins = geometryMargins + calcMargins(QSize(gridWidth * columnCount, gridHeight * rowCount), canvasSize);

    canvasInfo = CanvasInfo(columnCount, rowCount, gridWidth, gridHeight);
}

QMargins CanvasViewPrivate::calcMargins(const QSize &inSize, const QSize &outSize)
{
    auto horizontal = (outSize.width() - inSize.width());
    auto vertical = (outSize.height() - inSize.height());
    auto left = horizontal / 2;
    auto right = horizontal - left;
    auto top = vertical / 2;
    auto bottom = vertical - top;

    return QMargins(left, top, right, bottom);
}

QRect CanvasViewPrivate::visualRect(const QPoint &gridPos) const
{
    auto x = gridPos.x() * canvasInfo.gridWidth + viewMargins.left();
    auto y = gridPos.y() * canvasInfo.gridHeight + viewMargins.top();
    return QRect(x, y, canvasInfo.gridWidth, canvasInfo.gridHeight);
}

QRect CanvasViewPrivate::visualRect(const QString &item) const
{
    QPair<int, QPoint> pos;
    // query the point of item.
    // if not find, using overlap point instead.
    if (!GridIns->point(item, pos))
        pos.second = overlapPos();

    return visualRect(pos.second);
}

QString CanvasViewPrivate::visualItem(const QPoint &gridPos) const
{
    if (gridPos == overlapPos()) {
        auto overlap = GridIns->overloadItems(screenNum);
        if (!overlap.isEmpty())
            return overlap.last();
    }

    return GridIns->item(screenNum, gridPos);
}

bool CanvasViewPrivate::isWaterMaskOn()
{
    QGSettings desktopSettings("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
    if (desktopSettings.keys().contains("water-mask"))
        return  desktopSettings.get("water-mask").toBool();
    return true;
}
