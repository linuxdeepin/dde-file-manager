/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "private/canvasview_p.h"
#include "canvasitemdelegate.h"
#ifndef NEWGRID
#include "canvasgridmanager.h"
#else
#include "grid/canvasgrid.h"
#include "base/schemefactory.h"
#endif
#include "displayconfig.h"

#include <QPainter>
#include <QDebug>
#include <QScrollBar>
#include <QPaintEvent>

CanvasView::CanvasView(QWidget *parent)
    : QAbstractItemView(parent)
    , d(new CanvasViewPrivate())
{
    initUI();
}

QRect CanvasView::visualRect(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QRect();
}

QRect CanvasView::visualRect(const QPoint &gridPos)
{
    auto x = gridPos.x() * d->cellWidth + d->viewMargins.left();
    auto y = gridPos.y() * d->cellHeight + d->viewMargins.top();
    return QRect(x, y, d->cellWidth, d->cellHeight);
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

void CanvasView::setScreenName(const QString name)
{
    d->screenName = name;
}

int CanvasView::getScreenNum()
{
    return d->screenNum;
}

QString CanvasView::getScreenName()
{
    return d->screenName;
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
        updateCanvas();
        // todo:水印
    }
}

void CanvasView::updateCanvas()
{
    // todo:
    itemDelegate()->updateItemSizeHint();
    auto itemSize = itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex());
    QMargins geometryMargins = QMargins(0, 0, 0, 0);
    d->updateCanvasSize(this->geometry().size(), this->geometry().size(), geometryMargins, itemSize);
#ifndef NEWGRID
    CanvasGridManager::instance()->updateGridSize(d->screenNum, d->colCount, d->rowCount);
#else
    GridIns->updateSize(d->screenNum, QSize(d->colCount, d->rowCount));
#endif

}

QString CanvasView::fileDisplayNameRole(const QModelIndex &index)
{
    if (index.isValid())
        return index.data(CanvasModel::FileDisplayNameRole).toString();
    return QString();
}

void CanvasView::initUI()
{
    setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAutoFillBackground(false);
    setFrameShape(QFrame::NoFrame);   // TODO: using QWidget instead of QFrame?

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    setDefaultDropAction(Qt::CopyAction);

    // init icon delegate
    auto delegate = new CanvasItemDelegate(this);
    setItemDelegate(delegate);
    delegate->setIconLevel(DispalyIns->iconLevel());
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
#ifndef NEWGRID
    repaintLocalFiles = CanvasGridManager::instance()->items(d->screenNum);
#else
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

#endif
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
#ifndef NEWGRID
    auto overLapScreen = CanvasGridManager::instance()->overlapScreen();
    if (-1 == overLapScreen)
        return;
    auto overLapItems = CanvasGridManager::instance()->overlapItems();

    if (d->screenNum == overLapScreen) {
#else
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
#endif
        const QPoint overlapPos(d->colCount - 1, d->rowCount - 1);
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
    option.rect = visualRect(pos);
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
    Q_UNUSED(painter)
    d->debug_show_grid = true;
    if (d->debug_show_grid) {
        painter->save();
        if (canvasModel()) {
            for (int i = 0; i < d->colCount * d->rowCount; ++i) {
                // todo:CellMargins计算有点偏移
                auto pos = d->indexCoordinate(i).position();
                auto x = pos.x() * d->cellWidth + d->viewMargins.left();
                auto y = pos.y() * d->cellHeight + d->viewMargins.top();

                auto rect = QRect(x, y, d->cellWidth, d->cellHeight);

                int rowMode = pos.x() % 2;
                int colMode = pos.y() % 2;
                auto color = (colMode == rowMode) ? QColor(0, 0, 255, 32) : QColor(255, 0, 0, 32);
                painter->fillRect(rect, color);

                if (pos == d->dragTargetGrid) {
                    painter->fillRect(rect, Qt::green);
                }
                painter->setPen(QPen(Qt::red, 2));
                painter->drawText(rect, QString("%1-%2").arg(pos.x()).arg(pos.y()));
            }
        }
        painter->restore();
    }
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

    option.rect = visualRect(pos);

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
    Q_UNUSED(painter)
}

/*!
    文件拖动相关绘制，参数\a painter用于绘制， \a option拖动绘制项相关信息
*/
void CanvasView::drawDragMove(QPainter *painter, QStyleOptionViewItem &option)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
}
