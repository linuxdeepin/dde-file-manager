// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "displayconfig.h"
#include "view/canvasview_p.h"
#include "operator/boxselector.h"
#include "operator/viewpainter.h"
#include "delegate/canvasitemdelegate.h"
#include "grid/canvasgrid.h"
#include "displayconfig.h"
#include "operator/canvasviewmenuproxy.h"
#include "operator/fileoperatorproxy.h"
#include "utils/keyutil.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-framework/dpf.h>
#include <dfm-base/base/application/application.h>

#include <QPainter>
#include <QDebug>
#include <QScrollBar>
#include <QPaintEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QTimer>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

CanvasView::CanvasView(QWidget *parent)
    : QAbstractItemView(parent), d(new CanvasViewPrivate(this))
{
}

QRect CanvasView::visualRect(const QModelIndex &index) const
{
    auto item = model()->fileUrl(index).toString();
    QPoint gridPos;
    if (d->itemGridpos(item, gridPos))
        return d->visualRect(gridPos);

    // the index is not on this.
    return QRect();
}

void CanvasView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    Q_UNUSED(index)
    Q_UNUSED(hint);
}

QModelIndex CanvasView::indexAt(const QPoint &point) const
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
            // fmDebug() << "preesed on editor" << rowIndex;
            return rowIndex;
        }
    } else if (itemDelegate()->mayExpand(&rowIndex)) {   // second
        // get the expended rect.
        auto listRect = itemPaintGeomertys(rowIndex);
        if (checkRect(listRect, point)) {
            // fmDebug() << "preesed on expand index" << rowIndex;
            return rowIndex;
        }
    }

    return baseIndexAt(point);
}

QModelIndex CanvasView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers)

    QModelIndex current = currentIndex();
    if (!current.isValid()) {
        fmDebug() << "current index is invalid.";
        return d->firstIndex();
    }

    QPoint pos;
    {
        QPair<int, QPoint> postion;
        auto currentItem = model()->fileUrl(current).toString();
        if (Q_UNLIKELY(!GridIns->point(currentItem, postion))) {
            fmWarning() << "can not find pos for" << currentItem;
            return d->firstIndex();
        }

        if (Q_UNLIKELY(postion.first != screenNum())) {
            fmWarning() << currentItem << "item is not on" << screenNum() << postion.first;
            return d->firstIndex();
        }
        pos = postion.second;
    }

    GridCoordinate newCoord(pos);
    QString currentItem;

#define checkItem(pos, it)                \
    it = GridIns->item(screenNum(), pos); \
    if (!it.isEmpty())                    \
        break;

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

    // fmDebug() << "cursorAction" << cursorAction << "KeyboardModifiers" << modifiers << currentItem;
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
    // fmWarning() << "do not using this" << rect.normalized();
    return;

    //    QItemSelection selection;
    //    BoxSelIns->selection(this, rect.normalized(), &selection);
    //    selectionModel()->select(selection, command);
}

QRegion CanvasView::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion region;
    auto selectedList = selection.indexes();
    for (auto &index : selectedList)
        region = region.united(QRegion(visualRect(index)));

    return region;
}

void CanvasView::keyboardSearch(const QString &search)
{
    d->keySelector->keyboardSearch(search);
}

void CanvasView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    QItemSelectionModel *oldSelectionModel = QAbstractItemView::selectionModel();
    QAbstractItemView::setSelectionModel(selectionModel);
    if (oldSelectionModel)
        oldSelectionModel->deleteLater();
}

QList<QRect> CanvasView::itemPaintGeomertys(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    auto item = model()->fileUrl(index).toString();
    QPoint gridPos;
    if (!d->itemGridpos(item, gridPos))
        return {};

    QStyleOptionViewItem option;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    initViewItemOption(&option);
#else
    option = viewOptions();
#endif
    option.rect = d->itemRect(gridPos);
    return itemDelegate()->paintGeomertys(option, index);
}

QRect CanvasView::expendedVisualRect(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    QRect visRect;
    auto item = model()->fileUrl(index).toString();
    QPoint gridPos;
    if (!d->itemGridpos(item, gridPos))
        return visRect;

    visRect = d->visualRect(gridPos);

    QStyleOptionViewItem option;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    initViewItemOption(&option);
#else
    option = viewOptions();
#endif
    option.rect = d->itemRect(gridPos);
    option.rect = itemDelegate()->expendedGeomerty(option, index);

    // expend
    if (visRect.bottom() < option.rect.bottom())
        visRect.setBottom(option.rect.bottom());

    return visRect;
}

QVariant CanvasView::inputMethodQuery(Qt::InputMethodQuery query) const
{
    // When no item is selected, return input method area where the current mouse is located
    if (query == Qt::ImCursorRectangle && !currentIndex().isValid())
        return QRect(mapFromGlobal(QCursor::pos()), iconSize());

    return QAbstractItemView::inputMethodQuery(query);
}

WId CanvasView::winId() const
{
    // If it not the top widget and QAbstractItemView::winId() is called,that will cause errors in window system coordinates and graphics.
    if (isTopLevel()) {
        return QAbstractItemView::winId();
    } else {
        return topLevelWidget()->winId();
    }
}

QModelIndex CanvasView::baseIndexAt(const QPoint &point) const
{
    auto checkRect = [](const QList<QRect> &listRect, const QPoint &point) -> bool {
        if (listRect.isEmpty())
            return false;

        // icon rect
        QRect iconRect = listRect.at(0);
        if (iconRect.contains(point))
            return true;

        if (listRect.size() > 1) {
            QRect identify = listRect.at(1);
            if (identify.contains(point))
                return true;

            // 检查图标和文本的包围区域
            QRect combinedRect = iconRect.united(identify);
            return combinedRect.contains(point);
        }
        return false;
    };

    // then check the item on the point.
    QString item = d->visualItem(d->gridAt(point));
    auto rowIndex = model()->index(item, 0);
    if (!rowIndex.isValid())
        return rowIndex;

    auto listRect = itemPaintGeomertys(rowIndex);
    if (checkRect(listRect, point)) {
        // fmDebug() << "pressed on" << item << rowIndex;
        return rowIndex;
    }

    return QModelIndex();
}

void CanvasView::paintEvent(QPaintEvent *event)
{
    ViewPainter painter(d);
    painter.setRenderHint(QPainter::Antialiasing);

    // debug网格信息展示
    painter.drawGirdInfos();

    // 桌面文件绘制
    QStyleOptionViewItem option;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    initViewItemOption(&option);
#else
    option = viewOptions();
#endif

    // for flicker when refresh.
    if (!d->flicker) {
        // sort move
        painter.drawMove(option);
        // dodge
        painter.drawDodge(option);
        painter.paintFiles(option, event);
    }
}

void CanvasView::contextMenuEvent(QContextMenuEvent *event)
{
    if (CanvasViewMenuProxy::disableMenu())
        return;

    QPoint gridPos = d->gridAt(event->pos());
    itemDelegate()->revertAndcloseEditor();

    const QModelIndex &index = indexAt(event->pos());
    bool isEmptyArea = !index.isValid();
    Qt::ItemFlags flags;

    if (WindowUtils::isWayLand())
        setAttribute(Qt::WA_InputMethodEnabled, false);
    if (isEmptyArea) {
        d->menuProxy->showEmptyAreaMenu(flags, gridPos);
    } else {
        // menu focus is on the index that is not selected
        if (!selectionModel()->isSelected(index))
            selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);

        flags = model()->flags(index);
        d->menuProxy->showNormalMenu(index, flags, gridPos);
    }
    if (WindowUtils::isWayLand())
        setAttribute(Qt::WA_InputMethodEnabled, true);
}

void CanvasView::startDrag(Qt::DropActions supportedActions)
{
    if (d->viewSetting->isDelayDrag())
        return;

    // close editor before drag.
    // normally, items in editing status do not enter startDrag.
    // but if drag and drope one item before editing it, then draging it, startDrag will be called.
    // the reason is that when one item droped, the d->pressedIndex is setted to invaild.
    // then in mousePressEvent d->pressedIndex is not updating to preesed index because the state is EditingState.
    // finally, in mouseMoveEvent state is changed from EditingState to DragSelectingState because d->pressedInde is invaild.
    if (isPersistentEditorOpen(currentIndex()))
        closePersistentEditor(currentIndex());

    if (d->hookIfs && d->hookIfs->startDrag(screenNum(), supportedActions)) {
        fmDebug() << "start drag by extend.";
        return;
    }

    QModelIndexList validIndexes = selectionModel()->selectedIndexesCache();
    if (validIndexes.count() > 1) {
        fmDebug() << "Starting multi-item drag operation with" << validIndexes.count() << "items";
        QMimeData *data = model()->mimeData(validIndexes);
        if (!data)
            return;

        QPixmap pixmap = ViewPainter::polymerize(validIndexes, d);
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
    if (d->dragDropOper->enter(event))
        return;

    QAbstractItemView::dragEnterEvent(event);
}

void CanvasView::dragMoveEvent(QDragMoveEvent *event)
{
    if (d->dragDropOper->move(event))
        return;
    QAbstractItemView::dragMoveEvent(event);
}

void CanvasView::dragLeaveEvent(QDragLeaveEvent *event)
{
    d->dragDropOper->leave(event);
    QAbstractItemView::dragLeaveEvent(event);
}

void CanvasView::dropEvent(QDropEvent *event)
{
    if (d->dragDropOper->drop(event)) {
        activateWindow();
        setState(NoState);
        return;
    }
    QAbstractItemView::dropEvent(event);
}

void CanvasView::focusInEvent(QFocusEvent *event)
{
    QAbstractItemView::focusInEvent(event);

    // WA_InputMethodEnabled will be set to false if current index is invalid in QAbstractItemView::focusInEvent
    // To enable WA_InputMethodEnabled no matter whether the current index is valid or not.
    if (!testAttribute(Qt::WA_InputMethodEnabled))
        setAttribute(Qt::WA_InputMethodEnabled, true);
}

void CanvasView::focusOutEvent(QFocusEvent *event)
{
    d->dodgeOper->stopDelayDodge();
    d->dodgeOper->updatePrepareDodgeValue(event);
    QAbstractItemView::focusOutEvent(event);
}

void CanvasView::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::FontChange:
        updateGrid();
        break;
    default:
        break;
    }

    QAbstractItemView::changeEvent(event);
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

CanvasProxyModel *CanvasView::model() const
{
    return qobject_cast<CanvasProxyModel *>(QAbstractItemView::model());
}

CanvasSelectionModel *CanvasView::selectionModel() const
{
    return qobject_cast<CanvasSelectionModel *>(QAbstractItemView::selectionModel());
}

void CanvasView::setGeometry(const QRect &rect)
{
    if (rect.size().width() < 1 || rect.size().height() < 1) {
        fmWarning() << "Invalid geometry size - width:" << rect.size().width() << "height:" << rect.size().height();
        return;
    } else {
        fmDebug() << "Setting geometry for screen" << d->screenNum << "size:" << rect.size();
        QAbstractItemView::setGeometry(rect);
        updateGrid();

        if (d->waterMask)
            d->waterMask->updatePosition();
    }
}

void CanvasView::updateGrid()
{
    itemDelegate()->updateItemSizeHint();
    // close editor
    itemDelegate()->revertAndcloseEditor();

    auto itemSize = itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex());

    // add view margin. present is none.
    const QMargins geometryMargins = QMargins(0, 0, 0, 0);
    d->updateGridSize(geometry().size(), geometryMargins, itemSize);

    GridIns->updateSize(d->screenNum, QSize(d->canvasInfo.columnCount, d->canvasInfo.rowCount));

    //! reset timer to sync profile.
    //! we need to save current item pos after a while
    //! because there will be a wrong available geometry at screen or mode changes.
    GridIns->requestSync(2000);

    update();
}

void CanvasView::showGrid(bool v) const
{
    d->showGrid = v;
}

void CanvasView::aboutToResortFiles()
{
    if (!d->sortAnimOper)
        return;

    QStringList existItems;
    const QList<QUrl> &actualList = model()->files();
    for (const QUrl &df : actualList)
        existItems.append(df.toString());

    d->sortAnimOper->setMoveValue(existItems);
}

void CanvasView::filesResorted()
{
    if (!d->sortAnimOper)
        return;

    d->sortAnimOper->tryMove();
}

void CanvasView::refresh(bool silent)
{
    fmDebug() << "Refreshing canvas view - silent:" << silent;

    model()->refresh(rootIndex(), true);

    // flicker
    if (!silent) {
        d->flicker = true;
        repaint();
        update();
        d->flicker = false;
    }

    dpfSignalDispatcher->publish(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), "signal_CanvasView_RequestRefresh", silent);
}

void CanvasView::reset()
{
    QAbstractItemView::reset();
    // the reset will be called on model()->endResetModel().
    // all data and state will be cleared in QAbstractItemView::reset.
    // it need to reset root index there.
    setRootIndex(model()->rootIndex());
}

bool CanvasView::edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event)
{
    // only click on single selected index can be edited.
    if (selectionModel()->selectedRows().size() != 1)
        return false;

    // donot edit if ctrl or shift is pressed.
    if (isCtrlOrShiftPressed())
        return false;

    // check pressed on text area
    if (trigger == SelectedClicked) {
        auto list = itemPaintGeomertys(index);
        if (list.size() >= 2) {
            if (!list.at(1).contains(static_cast<QMouseEvent *>(event)->pos()))
                return false;
        }
    }

    return QAbstractItemView::edit(index, trigger, event);
}

void CanvasView::selectAll()
{
#if 0   // only select all item that on this view.
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
    selectionModel()->selectAll();
#endif
}

void CanvasView::toggleSelect()
{
    d->keySelector->toggleSelect();
}

void CanvasView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QAbstractItemView::currentChanged(current, previous);

    // WA_InputMethodEnabled will be set to false if current index is invalid in QAbstractItemView::currentChanged
    // To enable WA_InputMethodEnabled no matter whether the current index is valid or not.
    if (!testAttribute(Qt::WA_InputMethodEnabled))
        setAttribute(Qt::WA_InputMethodEnabled, true);
}

void CanvasView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // update selected and deselected item
    QAbstractItemView::selectionChanged(selected, deselected);

    // update selection state.
    d->operState().selectionChanged(selected, deselected);
}

QRect CanvasView::itemRect(const QModelIndex &index) const
{
    auto item = model()->fileUrl(index).toString();
    QPoint gridPos;
    if (d->itemGridpos(item, gridPos))
        return d->itemRect(gridPos);

    return QRect();
}

void CanvasView::keyPressEvent(QKeyEvent *event)
{
    if (d->hookIfs->keyPress(screenNum(), event->key(), event->modifiers()))
        return;

    if (d->keySelector->filterKeys().contains(static_cast<Qt::Key>(event->key()))) {
        d->keySelector->keyPressed(event);
        return;
    } else if (d->shortcutOper->keyPressed(event)) {
        return;
    }

    QAbstractItemView::keyPressEvent(event);
}

void CanvasView::mousePressEvent(QMouseEvent *event)
{
    if (d->hookIfs->mousePress(screenNum(), event->button(), event->pos()))
        return;

    // must get index on pos before QAbstractItemView::mousePressEvent
    auto index = indexAt(event->pos());
    d->viewSetting->checkTouchDrag(event);
    QAbstractItemView::mousePressEvent(event);

    if (event->button() == Qt::LeftButton) {
        if (itemDelegate())
            itemDelegate()->commitDataAndCloseEditor();

        if (!index.isValid()) {   // empty area
            BoxSelIns->beginSelect(event->globalPos(), true);
            setState(DragSelectingState);
        }
    }

    d->clickSelector->click(index);
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
    d->clickSelector->release(releaseIndex);
}

void CanvasView::wheelEvent(QWheelEvent *event)
{
    {
        QVariantHash ext;
        ext.insert("QWheelEvent", (qlonglong)event);
        ext.insert("CtrlPressed", isCtrlPressed());
        if (d->hookIfs && d->hookIfs->wheel(screenNum(), event->angleDelta(), &ext))
            return;
    }

    if (isCtrlPressed()) {
        d->menuProxy->changeIconLevel(event->angleDelta().y() > 0);
        event->accept();
    }
}

void CanvasView::initUI()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_InputMethodEnabled);
    viewport()->setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAutoFillBackground(false);
    setFrameShape(QFrame::NoFrame);

    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    setDefaultDropAction(Qt::CopyAction);

    // init icon delegate
    auto delegate = new CanvasItemDelegate(this);
    setItemDelegate(delegate);
    delegate->setIconLevel(DispalyIns->iconLevel());

    Q_ASSERT(selectionModel());
    d->operState().setView(this);
    Q_ASSERT(model());
    setRootIndex(model()->rootIndex());

    // water mask
    if (d->isWaterMaskOn() && WatermaskContainer::isEnable()) {
        Q_ASSERT(!d->waterMask);
        d->waterMask = new WatermaskContainer(this);
        d->waterMask->refresh();
    }
}

void CanvasView::setViewHook(ViewHookInterface *ext)
{
    d->hookIfs = ext;
}

ViewHookInterface *CanvasView::viewHook() const
{
    return d->hookIfs;
}

const QMargins CanvasViewPrivate::gridMiniMargin = QMargins(2, 2, 2, 2);

// dockReserveSize leads to lessen the column and row，and increase the width and height of grid by expanding grid margin.
// keep it for compatibility. Remove it if need reduce the grid margin
const QSize CanvasViewPrivate::dockReserveSize = QSize(80, 80);

CanvasViewPrivate::CanvasViewPrivate(CanvasView *qq)
    : QObject(qq), q(qq)
{
    // using NoSelection to turn off selection of QAbstractItemView
    // and CanvasView will to do selection by its selector.
    q->setSelectionMode(QAbstractItemView::NoSelection);
    q->setSelectionBehavior(QAbstractItemView::SelectItems);
    clickSelector = new ClickSelector(q);
    keySelector = new KeySelector(q);

    dragDropOper = new DragDropOper(q);
    dodgeOper = new DodgeOper(q);
    sortAnimOper = new SortAnimationOper(q);
    shortcutOper = new ShortcutOper(q);
    menuProxy = new CanvasViewMenuProxy(q);
    viewSetting = new ViewSettingUtil(q);

#ifdef QT_DEBUG
    showGrid = true;
#endif

    using namespace std::placeholders;
    connect(q, &CanvasView::clicked, this, std::bind(&CanvasViewPrivate::openIndexByClicked, this, ClickedAction::kClicked, _1));
    connect(q, &CanvasView::doubleClicked, this, std::bind(&CanvasViewPrivate::openIndexByClicked, this, ClickedAction::kDoubleClicked, _1));
}

CanvasViewPrivate::~CanvasViewPrivate()
{
    clickSelector = nullptr;
}

void CanvasViewPrivate::updateGridSize(const QSize &viewSize, const QMargins &geometryMargins, const QSize &itemSize)
{
    // canvas size is view size minus geometry margins.
    QSize canvasSize(viewSize.width() - geometryMargins.left() - geometryMargins.right(),
                     viewSize.height() - geometryMargins.top() - geometryMargins.bottom());
    fmInfo() << "view size" << viewSize << "canvas size" << canvasSize << "view margin" << geometryMargins << "item size" << itemSize;

    if (canvasSize.width() < 1 || canvasSize.height() < 1) {
        fmCritical() << "canvas size is invalid.";
        return;
    }

    // the minimum width of each grid.
    const int miniGridWidth = itemSize.width() + gridMiniMargin.left() + gridMiniMargin.right();

    // mins dockReserveSize is to keep column count same as the old.
    // it leads to fewer column count and widen the grid margin.
    int columnCount = (canvasSize.width() - dockReserveSize.width()) / miniGridWidth;
    int gridWidth = 1;
    if (Q_UNLIKELY(columnCount < 1)) {
        fmCritical() << " column count is 0. set it to 1 and set grid width to " << canvasSize.width();
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
    int rowCount = (canvasSize.height() - dockReserveSize.height()) / miniGridHeight;
    if (Q_UNLIKELY(rowCount < 1)) {
        fmCritical() << "row count is 0. set it to 1 and set grid height to" << canvasSize.height();
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

    fmInfo() << "grid size change from" << QSize(canvasInfo.columnCount, canvasInfo.rowCount) << "to" << QSize(columnCount, rowCount);
    canvasInfo = CanvasInfo(columnCount, rowCount, gridWidth, gridHeight);
}

QMargins CanvasViewPrivate::calcMargins(const QSize &inSize, const QSize &outSize)
{
    auto horizontal = (outSize.width() - inSize.width());
    auto vertical = (outSize.height() - inSize.height());
    horizontal = horizontal > 0 ? horizontal / 2 : 0;
    vertical = vertical > 0 ? vertical / 2 : 0;
    return QMargins(horizontal, vertical, horizontal, vertical);
}

QRect CanvasViewPrivate::visualRect(const QPoint &gridPos) const
{
    auto x = gridPos.x() * canvasInfo.gridWidth + viewMargins.left();
    auto y = gridPos.y() * canvasInfo.gridHeight + viewMargins.top();
    return QRect(x, y, canvasInfo.gridWidth, canvasInfo.gridHeight);
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

bool CanvasViewPrivate::itemGridpos(const QString &item, QPoint &gridPos) const
{
    if (item.isEmpty())
        return false;

    QPair<int, QPoint> pos;
    if (GridIns->point(item, pos)) {
        if (pos.first == screenNum) {
            gridPos = pos.second;
            return true;
        } else {
            // item is not on this view.
            return false;
        }
    }

    // check overlap
    if (GridIns->overloadItems(screenNum).contains(item)) {
        gridPos = overlapPos();
        return true;
    }

    return false;
}

bool CanvasViewPrivate::isWaterMaskOn()
{
    return true;
}

void CanvasViewPrivate::openIndexByClicked(const ClickedAction action, const QModelIndex &index)
{
    ClickedAction configAction = static_cast<ClickedAction>(Application::instance()->appAttribute(Application::kOpenFileMode).toInt());
    if (action == configAction) {
        Qt::ItemFlags flags = q->model()->flags(index);
        if (!flags.testFlag(Qt::ItemIsEnabled))
            return;

        if (!WindowUtils::keyCtrlIsPressed() && !WindowUtils::keyShiftIsPressed())
            openIndex(index);
    }
}

void CanvasViewPrivate::openIndex(const QModelIndex &index)
{
    const FileInfoPointer &info = q->model()->fileInfo(index);

    if (!info) {
        fmWarning() << "Cannot open index: file info is null";
        return;
    }

    QUrl fileUrl = info->urlOf(UrlInfoType::kUrl);
    fmDebug() << "Opening file:" << fileUrl;
    FileOperatorProxyIns->openFiles(q, { fileUrl });
}

QModelIndex CanvasViewPrivate::findIndex(const QString &key, bool matchStart, const QModelIndex &current, bool reverseOrder, bool excludeCurrent) const
{
    int start = 0;
    if (current.isValid()) {
        QPoint gridPos = gridAt(q->visualRect(current).center());
        start = gridIndex(gridPos);
    }
    const int gridCount = canvasInfo.gridCount();
    for (int i = excludeCurrent ? 1 : 0; i < gridCount; ++i) {
        int next = reverseOrder ? gridCount + start - i : start + i;
        next = next % gridCount;
        if (excludeCurrent && next == start)
            continue;

        auto item = visualItem(gridCoordinate(next).point());
        QModelIndex index = q->model()->index(item);
        if (!index.isValid())
            continue;

        const QString &pinyinName = q->model()->data(index, Global::ItemRoles::kItemFilePinyinNameRole).toString();

        if (matchStart ? pinyinName.startsWith(key, Qt::CaseInsensitive)
                       : pinyinName.contains(key, Qt::CaseInsensitive)) {
            return index;
        }
    }

    return QModelIndex();
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
