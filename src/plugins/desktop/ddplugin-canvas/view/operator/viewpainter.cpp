// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "viewpainter.h"
#include "grid/canvasgrid.h"
#include "boxselector.h"

#include <dfm-base/base/schemefactory.h>

#include <QPainterPath>

using namespace ddplugin_canvas;

ViewPainter::ViewPainter(CanvasViewPrivate *dd)
    : QPainter(dd->q->viewport()), d(dd)
{
}

/*!
    待显示文件过滤和绘制，包括堆叠文件绘制，传入参数\a painter用于绘制，\a option绘制项相关信息，
    \a event绘制事件信息(包括重叠区域、待更新区域等信息)
*/
void ViewPainter::paintFiles(QStyleOptionViewItem option, QPaintEvent *event)
{
    // all item paint in drawMove
    if (d->sortAnimOper->getMoveAnimationing()) {
        fmDebug() << "Skipping normal file painting - move animation in progress";
        return;
    }

    QRect repaintRect = event->rect();
    QVector<QRect> region;
    const QRegion &eventRegion = event->region();
    for (auto it = eventRegion.begin(); it != eventRegion.end(); ++it) {
        region.append(*it);
    }
    QPair<QModelIndex, QPoint> expandItem;
    // if need expand.
    expandItem.second = QPoint(-1, -1);

    // item may need expand.
    // the expand item need to draw at last. otherwise other item will overlap the expeand text.
    itemDelegate()->mayExpand(&expandItem.first);

    // todo:封装优化代码
    {
        const QHash<QString, QPoint> &pos = GridIns->points(d->screenNum);
        bool dodgeAnimationing = d->dodgeOper->getDodgeAnimationing();
        const QStringList &dodgeItems = d->dodgeOper->getDodgeItems();
        for (auto itor = pos.begin(); itor != pos.end(); ++itor) {
            if (dodgeAnimationing && dodgeItems.contains(itor.key()))
                continue;

            auto index = model()->index(itor.key());
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
    {
        auto overlapPos = d->overlapPos();
        auto overlap = GridIns->overloadItems(d->screenNum);
        for (auto itor = overlap.begin(); itor != overlap.end(); ++itor) {
            option.rect = d->itemRect(overlapPos);
            auto index = model()->index(*itor);
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
    // painting extend.
    if (d->hookIfs && d->hookIfs->drawFile(d->screenNum, d->q->model()->fileUrl(index), this, &option))
        return;

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
                auto geos = view()->itemPaintGeomertys(index);
                for (int j = 0; j < geos.size(); ++j) {
                    save();
                    setPen(QPen(Qt::GlobalColor(Qt::red + j), 1));
                    drawRect(geos.at(j));
                    restore();
                }
            }
        }
        restore();
    }

    restore();
}

/*!
    让位相关绘制， \a option拖动绘制项相关信息
*/
void ViewPainter::drawDodge(QStyleOptionViewItem option)
{
    if (d->dodgeOper->getPrepareDodge()) {
        auto hoverIndex = d->dragDropOper->hoverIndex();
        auto url = model()->fileUrl(hoverIndex);
        auto selects = selectionModel()->selectedUrls();

        if (selects.contains(url) || (d->dodgeOper->getDodgeAnimationing() && d->dodgeOper->getDodgeItems().contains(url.toString()))) {
            // hover item selected,it draged.
            // or,it dodge animationing.
        } else if (hoverIndex.isValid() && hoverIndex != view()->currentIndex()) {
            QPainterPath path;
            QPen pen(QColor(30, 126, 255, 255 * 2 / 10));

            const int border = 1;
            pen.setWidth(1);
            auto lastRect = view()->visualRect(hoverIndex).marginsRemoved(QMargins(border, border, border, border));
            path.addRoundedRect(lastRect, 4, 4);
            fillPath(path, QColor(43, 167, 248, 255 * 3 / 10));
            strokePath(path, pen);
        }
    }

    if (d->dodgeOper->getDodgeAnimationing()) {
        const QStringList &dodgeItems = d->dodgeOper->getDodgeItems();
        for (auto animationItem : dodgeItems) {
            auto index = model()->index(animationItem);
            auto margins = view()->d->gridMargins;

            if (!index.isValid())
                continue;

            GridPos gridPos;
            if (!d->dodgeOper->getDodgeItemGridPos(animationItem, gridPos))
                continue;

            if (gridPos.first != view()->screenNum())
                continue;

            QRect end = view()->d->visualRect(gridPos.second).marginsRemoved(margins);
            auto tempCurrent = d->dodgeOper->getDodgeDuration();
            option.rect = view()->visualRect(index).marginsRemoved(margins);

            auto nx = option.rect.x() + (end.x() - option.rect.x()) * tempCurrent;
            auto ny = option.rect.y() + (end.y() - option.rect.y()) * tempCurrent;
            option.rect.setX(static_cast<int>(nx));
            option.rect.setY(static_cast<int>(ny));
            option.rect.setSize(end.size());

            save();
            drawFile(option, index, gridPos.second);
            restore();
        }
    }
}

void ViewPainter::drawMove(QStyleOptionViewItem option)
{
    if (d->sortAnimOper->getMoveAnimationing()) {
        const QStringList &moveItems = d->sortAnimOper->getMoveItems();
        qreal scale = view()->devicePixelRatioF();
        for (auto animationItem : moveItems) {
            auto index = model()->index(animationItem);
            auto margins = view()->d->gridMargins;

            if (!index.isValid())
                continue;

            GridPos gridPos;
            if (!d->sortAnimOper->getMoveItemGridPos(animationItem, gridPos))
                continue;

            if (gridPos.first != view()->screenNum())
                continue;

            QRect end = view()->d->visualRect(gridPos.second).marginsRemoved(margins);
            auto tempCurrent = d->sortAnimOper->getMoveDuration();
            option.rect = view()->visualRect(index).marginsRemoved(margins);

            auto nx = option.rect.x() + (end.x() - option.rect.x()) * tempCurrent;
            auto ny = option.rect.y() + (end.y() - option.rect.y()) * tempCurrent;
            option.rect.setX(static_cast<int>(nx));
            option.rect.setY(static_cast<int>(ny));
            option.rect.setSize(end.size());

            QPixmap itemPix = d->sortAnimOper->findPixmap(animationItem);
            if (itemPix.isNull()) {
                auto pixWidth = end.size().width() * scale;
                auto pixHeight = end.size().height() * scale;
                itemPix = QPixmap(QSize(qRound(pixWidth), qRound(pixHeight)));
                itemPix.setDevicePixelRatio(scale);
                itemPix.fill(Qt::transparent);

                auto opt = option;
                opt.rect.moveTo(0, 0);

                drawFileToPixmap(&itemPix, opt, index);

                d->sortAnimOper->setItemPixmap(animationItem, itemPix);
            }

            drawPixmap(QPointF(nx, ny), itemPix);
        }
    }
}

void ViewPainter::drawFileToPixmap(QPixmap *pix,
                                   QStyleOptionViewItem option,
                                   const QModelIndex &index)
{
    QPainter painter(pix);
    // painting extend.
    if (d->hookIfs && d->hookIfs->drawFile(d->screenNum, d->q->model()->fileUrl(index), &painter, &option))
        return;

    itemDelegate()->paint(&painter, option, index);
}

#if 0   // do draw selected rect by view. see BoxSelecter::updateRubber()
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
#endif

QPixmap ViewPainter::polymerize(QModelIndexList indexs, CanvasViewPrivate *d)
{
    if (indexs.isEmpty() || !d) {
        fmWarning() << "Cannot create drag pixmap - invalid parameters:" << indexs.isEmpty() << (!d);
        return QPixmap();
    }

    auto viewPtr = d->q;
    // get foucs item to set it on top.
    auto foucs = d->operState().current();
    if (!foucs.isValid()) {
        fmWarning() << "current index is invalid.";
        foucs = indexs.first();
    } else if (!indexs.contains(foucs)) {
        fmWarning() << "current index is not in indexs.";
        foucs = indexs.first();
    }
    const int indexCount = indexs.count();
    // remove focus which will paint on top
    indexs.removeAll(foucs);

    static const int iconWidth = 128;
    static const int iconMargin = 30;   // add margin for showing ratoted item.
    static const int maxIconCount = 4;   // max painting item number.
    static const int maxTextCount = 99;   // max text number.
    static const qreal rotateBase = 10.0;
    static const qreal opacityBase = 0.1;
    static const int rectSzie = iconWidth + iconMargin * 2;
    const qreal scale = viewPtr->devicePixelRatioF();

    QRect pixRect(0, 0, rectSzie, rectSzie);
    QPixmap pixmap(pixRect.size() * scale);
    pixmap.setDevicePixelRatio(scale);
    pixmap.fill(Qt::transparent);

    const qreal offsetX = pixRect.width() / 2;
    const qreal offsetY = pixRect.height() / 2;
    const QSize iconSize(iconWidth, iconWidth);

    QStyleOptionViewItem option;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    viewPtr->initViewItemOption(&option);
#else
    option = viewPtr->viewOptions();
#endif
    option.state |= QStyle::State_Selected;
    // icon rect in pixmap.
    option.rect = pixRect.translated(iconMargin, iconMargin);
    option.rect.setSize(iconSize);

    QPainter painter(&pixmap);
    // paint items except focus
    for (int i = qMin(maxIconCount - 1, indexs.count() - 1); i >= 0; --i) {
        painter.save();

        // opacity 50% 40% 30% 20%
        painter.setOpacity(1.0 - (i + 5) * opacityBase);

        // rotate
        {
            qreal rotate = rotateBase * (qRound((i + 1.0) / 2.0) / 2.0 + 1.0) * (i % 2 == 1 ? -1 : 1);
            auto tf = painter.transform();

            // rotate on center
            tf = tf.translate(offsetX, offsetY).rotate(rotate).translate(-offsetX, -offsetY);
            painter.setTransform(tf);
        }

        // paint icon
        viewPtr->itemDelegate()->paintDragIcon(&painter, option, indexs.at(i));

        painter.restore();
    }

    // paint focus
    QSize topIconSize;
    {
        painter.save();
        painter.setOpacity(0.8);
        topIconSize = viewPtr->itemDelegate()->paintDragIcon(&painter, option, foucs);
        painter.restore();
    }

    // paint text
    {
        int length = 0;
        QString text;
        if (indexCount > maxTextCount) {
            length = 28;   // there are three characters showed.
            text = QString::number(maxTextCount).append("+");
        } else {
            length = 24;   // one or two characters
            text = QString::number(indexCount);
        }

        // the text rect is on right bottom of top icon.
        // using actual size of top icon to calc postion.
        int x = iconMargin + (iconWidth + topIconSize.width() - length) / 2;
        int y = iconMargin + (iconWidth + topIconSize.height() - length) / 2;
        QRect textRect(x, y, length, length);

        // paint text background.
        drawEllipseBackground(&painter, textRect);
        drawDragText(&painter, text, textRect);
    }

    return pixmap;
}

void ViewPainter::drawDragText(QPainter *painter, const QString &str, const QRect &rect)
{
    painter->save();
    painter->setPen(Qt::white);
    // the font defined by ui ：Arial，12px, Bold
    QFont ft("Arial");
    ft.setPixelSize(12);
    ft.setBold(true);
    painter->setFont(ft);
    painter->drawText(rect, Qt::AlignCenter, str);
    painter->restore();
}

void ViewPainter::drawEllipseBackground(QPainter *painter, const QRect &rect)
{
    painter->save();
    QColor pointColor(244, 74, 74);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setOpacity(1);
    painter->setPen(pointColor);
    painter->setBrush(pointColor);
    painter->drawEllipse(rect);
    painter->restore();
}
