/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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
#include <QMenu>
#include <QDrag>
#include <QMimeData>

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
    Q_UNUSED(modifiers)

    QModelIndex current = currentIndex();
    if (!current.isValid()) {
        qDebug() << "current index is invalid.";
        return d->firstIndex();
    }

    QPoint pos;
    {
        QPair<int, QPoint> postion;
        auto currentItem = model()->url(current).toString();
        if (Q_UNLIKELY(!GridIns->point(currentItem, postion))) {
            qWarning() << "can not find pos for" << currentItem;
            return d->firstIndex();
        }

        if (Q_UNLIKELY(postion.first != screenNum())) {
            qWarning() << currentItem << "item is not on" << screenNum() << postion.first;
            return d->firstIndex();
        }
        pos = postion.second;
    }

    GridCoordinate newCoord(pos);
    QString currentItem;

#define checkItem(pos, it) \
    it = GridIns->item(screenNum(), pos);\
    if (!it.isEmpty()) \
        break

    switch (cursorAction) {
    case MoveLeft:
        while (pos.x() >= 0) {
            newCoord = newCoord.moveLeft();
            pos = newCoord.point();
            checkItem(pos, currentItem);
        }
        break;
    case MoveRight:
        while (pos.x() < d->canvasInfo.gridWidth) {
            newCoord = newCoord.moveRight();
            pos = newCoord.point();
            checkItem(pos, currentItem);
        }
        break;
    case MovePrevious:
    case MoveUp:
        while (pos.y() >= 0 && pos.x() >= 0) {
            newCoord = newCoord.moveUp();
            pos = newCoord.point();
            if (pos.y() < 0) {
                newCoord = GridCoordinate(pos.x() - 1, d->canvasInfo.rowCount - 1);
                pos = newCoord.point();
            }
            checkItem(pos, currentItem);
        }
        break;
    case MoveNext:
    case MoveDown:
        while (pos.y() < d->canvasInfo.rowCount && pos.x() < d->canvasInfo.columnCount) {
            newCoord = newCoord.moveDown();
            pos = newCoord.point();
            if (pos.y() >= d->canvasInfo.rowCount) {
                newCoord = GridCoordinate(pos.x() + 1, 0);
                pos = newCoord.point();
            }
            checkItem(pos, currentItem);
        }
        break;
    case MoveHome:
    case MovePageUp:
        return d->firstIndex();
    case MoveEnd:
    case MovePageDown:
        return d->lastIndex();
    default:
         break;
    }

    if (pos == d->overlapPos())
        return d->lastIndex();

    qDebug() << "cursorAction" << cursorAction << "KeyboardModifiers" << modifiers << currentItem;
    return model()->index(currentItem);
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
    if (!index.isValid())
        return {};

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

    // for flicker when refresh.
    if (!d->flicker)
        painter.paintFiles(option, event);

    // 绘制选中区域
    painter.drawSelectRect();

    // todo: 拖动绘制
    painter.drawDragMove(option);
}

void CanvasView::contextMenuEvent(QContextMenuEvent *event)
{
    d->lastMenuGridPos = d->gridAt(event->pos());

    const QModelIndex &index = indexAt(event->pos());
    Qt::ItemFlags flags;

    if (d->isEmptyArea(event->pos())) {
        d->showEmptyAreaMenu(flags);
    } else {
        flags = model()->flags(index);
        d->showNormalMenu(index, flags);
    }
}

void CanvasView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList validIndexes = selectionModel()->selectedIndexes();
    if (validIndexes.count() > 1) {
        QMimeData *data = model()->mimeData(validIndexes);
        if (!data)
            return;

        QPixmap pixmap = ViewPainter::polymerize(validIndexes, d.get());
        QDrag *drag = new QDrag(this);
        drag->setPixmap(pixmap);
        drag->setMimeData(data);
        drag->setHotSpot(QPoint(static_cast<int>(pixmap.size().width() / (2 * pixmap.devicePixelRatio())),
                                static_cast<int>(pixmap.size().height() / (2 * pixmap.devicePixelRatio()))));
        Qt::DropAction dropAction = Qt::IgnoreAction;
        Qt::DropAction defaultDropAction = QAbstractItemView::defaultDropAction();
        if (defaultDropAction != Qt::IgnoreAction && (supportedActions & defaultDropAction))
            dropAction = defaultDropAction;
        else if (supportedActions & Qt::CopyAction && dragDropMode() != QAbstractItemView::InternalMove)
            dropAction = Qt::CopyAction;
        drag->exec(supportedActions, dropAction);
    } else {
        QAbstractItemView::startDrag(supportedActions);
    }
}

void CanvasView::dragEnterEvent(QDragEnterEvent *event)
{
    if (!d->dragDropOper->enter(event))
        return;

    QAbstractItemView::dragEnterEvent(event);
    qInfo() << __FUNCTION__ << event->possibleActions() << event->dropAction() << model()->mimeTypes();
}

void CanvasView::dragMoveEvent(QDragMoveEvent *event)
{
    if (!d->dragDropOper->move(event))
        return;
    qInfo() << __FUNCTION__ << event->dropAction();
    QAbstractItemView::dragMoveEvent(event);
}

void CanvasView::dragLeaveEvent(QDragLeaveEvent *event)
{
    d->dragDropOper->leave(event);
    QAbstractItemView::dragLeaveEvent(event);
}

void CanvasView::dropEvent(QDropEvent *event)
{
    setState(NoState);
    if (!d->dragDropOper->drop(event)) {
        activateWindow();
        return;
    }
    QAbstractItemView::dropEvent(event);
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

void CanvasView::refresh()
{
    model()->fetchMore(QModelIndex());

    // flicker
    d->flicker = true;
    repaint();
    update();
    d->flicker = false;
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

void CanvasView::selectAll()
{
#if 0
    QStringList items;
    items << GridIns->points(d->screenNum).keys();
    items << GridIns->overloadItems(d->screenNum);

    if (items.isEmpty())
        return;

    QItemSelection selection;
    auto m = model();
    for (const QString &item : items) {
        auto index = m->index(item);
        selection.append(QItemSelectionRange(index));
    }

    selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);

    // set focus to first index.
    {
        auto first = d->firstIndex();
        d->operState().setCurrent(first);
        d->operState().setContBegin(first);
    }

#else
    QItemSelection selection;
    auto m = model();
    for (int row = 0 ; row < m->rowCount(rootIndex()); ++row) {
        auto index = m->index(row, 0);
        if (index.isValid())
            selection.append(QItemSelectionRange(index));
    }
    selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
#endif
}

QRect CanvasView::itemRect(const QModelIndex &index) const
{
    return d->itemRect(model()->url(index).toString());
}

void CanvasView::keyPressEvent(QKeyEvent *event)
{
    // todo(zy) disable shortcuts ("ApplicationAttribute", "DisableDesktopShortcuts", false)
    // todo 触摸屏
    if (d->keySelecter->filterKeys().contains(static_cast<Qt::Key>(event->key()))) {
        d->keySelecter->keyPressed(event);
        return;
    } else if (d->shortcutOper->keyPressed(event)) {
        return;
    }

    QAbstractItemView::keyPressEvent(event);
}

void CanvasView::mousePressEvent(QMouseEvent *event)
{
    QAbstractItemView::mousePressEvent(event);

    auto index = indexAt(event->pos());
    if (!index.isValid() && event->button() == Qt::LeftButton) { //empty area
        BoxSelIns->beginSelect(event->globalPos(), true);
        setState(DragSelectingState);
    }

    d->clickSelecter->click(index);
}

void CanvasView::mouseMoveEvent(QMouseEvent *event)
{
    QAbstractItemView::mouseMoveEvent(event);
}

void CanvasView::mouseReleaseEvent(QMouseEvent *event)
{
    QAbstractItemView::mouseReleaseEvent(event);

    if (event->button() != Qt::LeftButton)
        return;

    auto releaseIndex = indexAt(event->pos());
    d->clickSelecter->release(releaseIndex);

    setState(NoState);
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

    Q_ASSERT(selectionModel());
    d->operState().setView(this);
    Q_ASSERT(model());
    setRootIndex(model()->rootIndex());
    d->shortcutOper->regShortcut();

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
    clickSelecter = new ClickSelecter(q);
    keySelecter = new KeySelecter(q);
    dragDropOper = new DragDropOper(q);
    shortcutOper = new ShortcutOper(q);
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

bool CanvasViewPrivate::isEmptyArea(const QPoint &pos) const
{
    const QModelIndex &index = q->indexAt(pos);
    if (index.isValid() && q->selectionModel()->isSelected(index)) {
        qDebug() << "not empty:" << pos << index.data().toString();
        return false;
    }
    // todo(wangcl) expand item geometry,and so on

    return true;
}

bool CanvasViewPrivate::isWaterMaskOn()
{
    QGSettings desktopSettings("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
    if (desktopSettings.keys().contains("water-mask"))
        return  desktopSettings.get("water-mask").toBool();
    return true;
}

void CanvasViewPrivate::showEmptyAreaMenu(const Qt::ItemFlags &indexFlags)
{
    Q_UNUSED(indexFlags)
    // todo menu

    // test code
    QMenu *tstMenu = new QMenu;
    QAction *tstAction = nullptr;

    tstAction = tstMenu->addAction(tr("create file"));
    connect(tstAction, &QAction::triggered, q, [=](){
        emit q->createFileByMenu(screenNum, lastMenuGridPos);

        QString path = q->model()->rootUrl().path();
        path += "/testFile.txt";

        QFile file(path);
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
        }
    });

    tstAction = tstMenu->addAction(tr("create folder"));
    connect(tstAction, &QAction::triggered, q, [=](){
        emit q->createFileByMenu(screenNum, lastMenuGridPos);

        QDir dir(q->model()->rootUrl().path());
        dir.mkdir("testFolder");
    });

    tstAction = tstMenu->addAction(tr("select all"));
    connect(tstAction, &QAction::triggered, q, &CanvasView::selectAll);

    tstAction = tstMenu->addAction(tr("refresh"));
    connect(tstAction, &QAction::triggered, q, &CanvasView::refresh);

    tstMenu->exec(QCursor::pos());
    delete tstMenu;
}

void CanvasViewPrivate::showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags)
{
    Q_UNUSED(index)
    Q_UNUSED(indexFlags)
    // todo menu

    // test code
    QMenu *tstMenu = new QMenu;
    QAction *tstAction = nullptr;

    tstAction = tstMenu->addAction(tr("open"));
    connect(tstAction, &QAction::triggered, q, [=](){

    });

    tstAction = tstMenu->addAction(tr("delete"));
    connect(tstAction, &QAction::triggered, q, [=](){

    });

    tstMenu->exec(QCursor::pos());
    delete tstMenu;
}

QModelIndex CanvasViewPrivate::firstIndex() const
{
    int count = GridIns->gridCount(screenNum);
    for (int i = 0; i < count; ++i) {
        auto item = GridIns->item(screenNum, gridCoordinate(i).point());
        if (!item.isEmpty()) {
            return q->model()->index(item);
        }
    }
    return QModelIndex();
}

QModelIndex CanvasViewPrivate::lastIndex() const
{
    auto overlop = GridIns->overloadItems(screenNum);
    if (!overlop.isEmpty())
        return q->model()->index(overlop.last());

    int count = GridIns->gridCount(screenNum);
    for (int i = count - 1; i >= 0; --i) {
        auto item = GridIns->item(screenNum, gridCoordinate(i).point());
        if (!item.isEmpty()) {
            return q->model()->index(item);
        }
    }

    return QModelIndex();
}
