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
#include "canvasitemdelegate.h"
#include "grid/canvasgrid.h"
#include "displayconfig.h"

#include "base/schemefactory.h"

#include <QGSettings>

#include <QPainter>
#include <QDebug>
#include <QScrollBar>
#include <QPaintEvent>


DSB_D_USE_NAMESPACE

CanvasView::CanvasView(QWidget *parent)
    : QAbstractItemView(parent)
    , d(new CanvasViewPrivate(this))
{

}

QRect CanvasView::visualRect(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QRect();
}

void CanvasView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint) {
    Q_UNUSED(index)
            Q_UNUSED(hint)
}

QModelIndex CanvasView::indexAt(const QPoint &point) const
{
    Q_UNUSED(point)
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

void CanvasView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) {
    Q_UNUSED(rect)
            Q_UNUSED(command)
}

QRegion CanvasView::visualRegionForSelection(const QItemSelection &selection) const
{
    Q_UNUSED(selection)
    return QRegion();
}

void CanvasView::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    painter.setRenderHints(QPainter::HighQualityAntialiasing);

    auto option = viewOptions();
    option.textElideMode = Qt::ElideMiddle;
    painter.setBrush(QColor(255, 0, 0, 0));

    // debug网格信息展示
    drawGirdInfos(&painter);

    // todo:让位
    drawDodge(&painter);

    // 桌面文件绘制
    fileterAndRepaintLocalFiles(&painter, option, event);

    // 绘制选中区域
    drawSelectRect(&painter);

    // todo: 拖动绘制
    drawDragMove(&painter, option);
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

CanvasModel *CanvasView::canvasModel() const
{
    return qobject_cast<CanvasModel *>(QAbstractItemView::model());
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

QString CanvasView::fileDisplayNameRole(const QModelIndex &index)
{
    if (index.isValid())
        return index.data(CanvasModel::FileDisplayNameRole).toString();
    return QString();
}

void CanvasView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QAbstractItemView::mousePressEvent(event);
        return;
    }

    auto index = indexAt(event->pos());
    if (!index.isValid())
        BoxSelIns->beginSelect(event->globalPos(), true);
}

void CanvasView::initUI()
{
    setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAutoFillBackground(false);
    setFrameShape(QFrame::NoFrame);

    setSelectionMode(QAbstractItemView::ExtendedSelection);
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

/*!
    待显示文件过滤和绘制，包括堆叠文件绘制，传入参数\a painter用于绘制，\a option绘制项相关信息，
    \a event绘制事件信息(包括重叠区域、待更新区域等信息)
*/
void CanvasView::fileterAndRepaintLocalFiles(QPainter *painter, QStyleOptionViewItem &option, QPaintEvent *event)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(event)

    const QStyle::State state = option.state;
    const bool enabled = (state & QStyle::State_Enabled) != 0;

    // todo:封装优化代码
    QHash<QPoint, DFMDesktopFileInfoPointer> repaintLocalFiles;
    {
        const QHash<QString, QPoint> &pos = GridIns->points(d->screenNum);
        for (auto itor = pos.begin(); itor != pos.end(); ++itor) {
            auto info = DFMBASE_NAMESPACE::InfoFactory::create<DefaultDesktopFileInfo>(itor.key());
            if (!info){
                qWarning() << "create file info failed" << itor.key();
                continue;
            }

            repaintLocalFiles.insert(itor.value(), info);
        }
    }

    if (repaintLocalFiles.isEmpty())
        return;

    // 非重叠部分(有重叠时包括重叠位置被覆盖的最底层图标)
    for (auto fileItr = repaintLocalFiles.begin(); fileItr != repaintLocalFiles.end(); ++fileItr) {
        // todo:暂不考虑拖拽

        auto needPaint = isRepaintFlash(option, event, fileItr.key());
        if (!needPaint)
            continue;
        drawLocalFile(painter, option, enabled, fileItr.key(), fileItr.value());
    }

    // 重叠图标绘制(不包括最底层被覆盖的图标)
    // todo暂时没考虑堆叠的栈情况；
    {
        QList<DFMDesktopFileInfoPointer> overlapItems;
        auto overlap = GridIns->overloadItems(d->screenNum);
        for (auto itor = overlap.begin(); itor != overlap.end(); ++itor) {
            auto info = DFMBASE_NAMESPACE::InfoFactory::create<DefaultDesktopFileInfo>(*itor);
            if (!info){
                qWarning() << "create file info failed" << *itor;
                continue;
            }

            overlapItems.append(info);
        }

        const QPoint overlapPos(d->canvasInfo.columnCount - 1, d->canvasInfo.rowCount - 1);
        for (auto &item : overlapItems) {
            // todo：拖拽让位的一些图标保持情况

            auto needPaint = isRepaintFlash(option, event, overlapPos);
            if (!needPaint)
                continue;
            drawLocalFile(painter, option, enabled, overlapPos, item);
        }
    }
}

/*!
 * \brief 指定布局坐标位置是否重绘刷新
 * \param option item样式信息
 * \param event 绘制事件
 * \param pos 指定布局坐标位置
 * \return 返回刷新与否，true:刷新；false,不刷新
 */
bool CanvasView::isRepaintFlash(QStyleOptionViewItem &option, QPaintEvent *event, const QPoint pos)
{
    option.rect = d->visualRect(pos);
    auto repaintRect = event->rect();
    // 刷新区域判定，跳过不刷新的区域
    bool needflash = false;
    for (auto &rr : event->region().rects()) {
        if (rr.intersects(option.rect)) {
            needflash = true;
            break;
        }
    }

    // 不需要刷新和重绘
    if (!needflash || !repaintRect.intersects(option.rect))
        return false;
    return true;
}

/*!
    绘制显示栅格信息。当debug_show_grid变量为true时绘制栅格信息，反之不绘制，传入参数\a painter用于绘制。
*/
void CanvasView::drawGirdInfos(QPainter *painter)
{
    if (!d->showGrid)
        return;

    painter->save();
    painter->setPen(QPen(Qt::red, 2));

    for (int i = 0; i < d->canvasInfo.columnCount * d->canvasInfo.rowCount; ++i) {
        // todo:CellMargins计算有点偏移
        auto pos = d->gridCoordinate(i);
        auto rect = d->visualRect(pos.point());

        int rowMode = pos.x() % 2;
        int colMode = pos.y() % 2;
        auto color = (colMode == rowMode) ? QColor(0, 0, 255, 32) : QColor(255, 0, 0, 32);
        painter->fillRect(rect, color);

        // drag target
        if (pos.point() == d->dragTargetGrid)
            painter->fillRect(rect, Qt::green);

        painter->drawText(rect, QString("%1-%2").arg(pos.x()).arg(pos.y()));

    }
    painter->restore();
}

/*!
    让位相关绘制，由成员变量startDodge控制，startDodge为true进行让位相关绘制，传入参数\a painter用于绘制。
*/
void CanvasView::drawDodge(QPainter *painter)
{
    Q_UNUSED(painter)
}

void CanvasView::drawLocalFile(QPainter *painter, QStyleOptionViewItem &option,
                                      bool enabled, const QPoint pos,
                                      const DFMDesktopFileInfoPointer &file)
{
    // todo：拖拽让位的一些图标保持情况

    option.rect = d->visualRect(pos);

    auto tempModel = qobject_cast<CanvasModel *>(QAbstractItemView::model());
    auto index = tempModel->index(file);
    if (!index.isValid())
        return;
    if (selectionModel() && selectionModel()->isSelected(index)) {
        option.state |= QStyle::State_Selected;
    }
    if (enabled) {
        // todo: to understand
        QPalette::ColorGroup cg;
        if ((model()->flags(index) & Qt::ItemIsEnabled) == 0) {
            option.state &= ~QStyle::State_Enabled;
            cg = QPalette::Disabled;
        } else {
            cg = QPalette::Normal;
        }
        option.palette.setCurrentColorGroup(cg);
    }

    // todo: focus item style set

    option.state &= ~QStyle::State_MouseOver;
    painter->save();

    // todo: debug 图标geomtry信息
    this->itemDelegate()->paint(painter, option, index);
    painter->restore();
}

/*!
    选择文件状态哦绘制，绘制鼠标左键框选蒙版，参数\a painter用于绘制。
*/
void CanvasView::drawSelectRect(QPainter *painter)
{
    // is selecting. isBeginFrom is to limit only select on single view.
    if (!BoxSelIns->isAcvite() || !BoxSelIns->isBeginFrom(this))
        return;

    QRect selectRect = BoxSelIns->validRect(this);
    if (selectRect.isValid()) {
        QStyleOptionRubberBand opt;
        opt.initFrom(this);
        opt.shape = QRubberBand::Rectangle;
        opt.opaque = false;
        opt.rect = selectRect;
        painter->save();
        style()->drawControl(QStyle::CE_RubberBand, &opt, painter);
        painter->restore();
    }
}

/*!
    文件拖动相关绘制，参数\a painter用于绘制， \a option拖动绘制项相关信息
*/
void CanvasView::drawDragMove(QPainter *painter, QStyleOptionViewItem &option)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
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
}

CanvasViewPrivate::~CanvasViewPrivate()
{

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

QRect CanvasViewPrivate::visualRect(const QPoint &gridPos)
{
    auto x = gridPos.x() * canvasInfo.gridWidth + viewMargins.left();
    auto y = gridPos.y() * canvasInfo.gridHeight + viewMargins.top();
    return QRect(x, y, canvasInfo.gridWidth, canvasInfo.gridHeight);
}

bool CanvasViewPrivate::isWaterMaskOn()
{
    QGSettings desktopSettings("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
    if (desktopSettings.keys().contains("water-mask"))
        return  desktopSettings.get("water-mask").toBool();
    return true;
}
