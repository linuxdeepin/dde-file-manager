/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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

#include "viewpainter.h"
#include "grid/canvasgrid.h"
#include "boxselecter.h"

#include "base/schemefactory.h"

DSB_D_USE_NAMESPACE

ViewPainter::ViewPainter(CanvasViewPrivate *dd)
    : QPainter(dd->q->viewport())
    , d(dd)
{

}

/*!
    待显示文件过滤和绘制，包括堆叠文件绘制，传入参数\a painter用于绘制，\a option绘制项相关信息，
    \a event绘制事件信息(包括重叠区域、待更新区域等信息)
*/
void ViewPainter::paintFiles(QStyleOptionViewItem option, QPaintEvent *event)
{
    QRect repaintRect = event->rect();
    QVector<QRect> region = event->region().rects();

    QPair<QModelIndex, QPoint> expandItem;
    // if need expand.
    expandItem.second = QPoint(-1, -1);

    // item may need expand.
    // the expand item need to draw at last. otherwise other item will override the expeand text.
    itemDelegate()->mayExpand(&expandItem.first);

    // todo:封装优化代码
    {
        const QHash<QString, QPoint> &pos = GridIns->points(d->screenNum);
        for (auto itor = pos.begin(); itor != pos.end(); ++itor) {
            auto info = DFMBASE_NAMESPACE::InfoFactory::create<DefaultDesktopFileInfo>(itor.key());
            if (!info){
                qWarning() << "create file info failed" << itor.key();
                continue;
            }

            auto index = model()->index(info);
            if (!index.isValid())
                continue;

            // item rect that can be used to draw. is need to move it to CanvasItemDelegate::initStyleOption?
            // using CanvasView::itemPaintRect to get rect
            option.rect = d->itemRect(itor.value());

            // expand item need to last draw.
            if (expandItem.first == index) {
                expandItem.second = itor.value();
            } else if (repaintRect.intersects(option.rect)) {
                for (auto &rr : region) {
                    if (rr.intersects(option.rect)) {
                        drawFile(option, index, itor.value());
                        break;
                    }
                }
            }
        }
    }

    // 重叠图标绘制(不包括最底层被覆盖的图标)
    // todo暂时没考虑堆叠的栈情况；
    {
        auto overlapPos = d->overlapPos();
        auto overlap = GridIns->overloadItems(d->screenNum);
        for (auto itor = overlap.begin(); itor != overlap.end(); ++itor) {
            auto info = DFMBASE_NAMESPACE::InfoFactory::create<DefaultDesktopFileInfo>(*itor);
            if (!info){
                qWarning() << "create file info failed" << *itor;
                continue;
            }

            option.rect = d->itemRect(overlapPos);
            auto index = model()->index(info);
            if (!index.isValid())
                continue;

            // expand item need to last draw.
            if (expandItem.first == index) {
                expandItem.second = overlapPos;
            } else if (repaintRect.intersects(option.rect)) {
                for (auto &rr : region) {
                    if (rr.intersects(option.rect)) {
                        drawFile(option, index, overlapPos);
                        break;
                    }
                }
            }
        }
    }

    // try to expand it if necessary
     if (expandItem.first.isValid() && expandItem.second.x() > -1 && Q_LIKELY(expandItem.second.y() > -1)) {
        option.rect = d->itemRect(expandItem.second);
        drawFile(option, expandItem.first, expandItem.second);
     }
}

void ViewPainter::drawFile(QStyleOptionViewItem option, const QModelIndex &index, const QPoint &gridPos)
{
    Q_UNUSED(gridPos);
    itemDelegate()->paint(this, option, index);
}

/*!
    绘制显示栅格信息。当debug_show_grid变量为true时绘制栅格信息，反之不绘制，传入参数\a painter用于绘制。
*/
void ViewPainter::drawGirdInfos()
{
    if (!d->showGrid)
        return;

    save();

    QPen pen;
    pen.setColor(Qt::black);
    pen.setStyle(Qt::DotLine);
    pen.setWidth(1);
    setPen(pen);
    setBrush(Qt::NoBrush);

    for (int i = 0; i < d->canvasInfo.gridCount(); ++i) {
        auto pos = d->gridCoordinate(i);
        auto rect = d->visualRect(pos.point());

        // gird background
        auto backColor = ((pos.x() % 2) == (pos.y() % 2)) ? QColor(0, 0, 255, 32) : QColor(255, 0, 0, 32);
        fillRect(rect, backColor);

        // drag target
        if (pos.point() == d->dragTargetGrid)
            fillRect(rect, Qt::green);

        // draw grid border.
        drawRect(rect);

       // draw serial number
       save();
       setPen(QPen(Qt::red, 2));
       drawText(rect, QString("%1-%2").arg(pos.x()).arg(pos.y()));

       setPen(QPen(Qt::red, 1));
       // draw paint geomertys of the item that on the pos.
       auto item = GridIns->item(d->screenNum, pos.point());
       if (!item.isEmpty()) {
           QModelIndex index = model()->index(item, 0);
           // draw item rect
           drawRect(d->itemRect(pos.point()));

           if (index.isValid()) {
               for (auto rect : view()->itemPaintGeomertys(index)) {
                   drawRect(rect);
               }
           }
       }
       restore();
    }

    restore();
}

/*!
    让位相关绘制，由成员变量startDodge控制，startDodge为true进行让位相关绘制，传入参数\a painter用于绘制。
*/
void ViewPainter::drawDodge()
{

}

/*!
    选择文件状态哦绘制，绘制鼠标左键框选蒙版，参数\a painter用于绘制。
*/
void ViewPainter::drawSelectRect()
{
    // is selecting. isBeginFrom is to limit only select on single view.
    if (!BoxSelIns->isAcvite() || !BoxSelIns->isBeginFrom(view()))
        return;

    QRect selectRect = BoxSelIns->validRect(view());
    if (selectRect.isValid()) {
        QStyleOptionRubberBand opt;
        opt.initFrom(view());
        opt.shape = QRubberBand::Rectangle;
        opt.opaque = false;
        opt.rect = selectRect;
        save();
        view()->style()->drawControl(QStyle::CE_RubberBand, &opt, this);
        restore();
    }
}

/*!
    文件拖动相关绘制，参数\a painter用于绘制， \a option拖动绘制项相关信息
*/
void ViewPainter::drawDragMove(QStyleOptionViewItem &option)
{
    Q_UNUSED(option)
}
