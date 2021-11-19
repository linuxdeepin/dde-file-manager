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
#ifndef DEFAULTCANVASVIEW_P_H
#define DEFAULTCANVASVIEW_P_H

#include "defaultcanvasview.h"

#include <QDebug>

class Coordinate
{
public:
    Coordinate(int _x, int _y)
    {
        d.position.x = _x;
        d.position.y = _y;
    }

    explicit Coordinate(QPoint pos)
    {
        d.position.x = pos.x();
        d.position.y = pos.y();
    }

    QPoint position()
    {
        return QPoint(d.position.x, d.position.y);
    }

    Coordinate moveLeft(int offset = 1) const
    {
        return Coordinate(d.position.x - offset, d.position.y);
    }

    Coordinate moveRight(int offset = 1) const
    {
        return Coordinate(d.position.x + offset, d.position.y);
    }

    Coordinate moveUp(int offset = 1) const
    {
        return Coordinate(d.position.x, d.position.y - offset);
    }

    Coordinate moveDown(int offset = 1) const
    {
        return Coordinate(d.position.x, d.position.y + offset);
    }

private:
    union CoordinateData {
        struct
        {
            int32_t x;
            int32_t y;
        } position;
        // todo:CoordValue value为何被屏蔽，之前作何之用？
        // CoordValue value;
    };

    CoordinateData d;
};

class DefaultCanvasViewPrivate
{
public:
    explicit DefaultCanvasViewPrivate() {}
    ~DefaultCanvasViewPrivate() {}

    void updateCellMargins(QMargins &tgMargin, const QSize &szItem, const QSize &szCell)
    {
        auto horizontalMargin = (szCell.width() - szItem.width());
        auto verticalMargin = (szCell.height() - szItem.height());
        auto leftMargin = horizontalMargin / 2;
        auto rightMargin = horizontalMargin - leftMargin;
        auto topMargin = verticalMargin / 2;
        auto bottom = verticalMargin - topMargin;

        tgMargin = QMargins(leftMargin, topMargin, rightMargin, bottom);
    }

    void updateCanvasSize(const QSize &szSceeen, const QSize &szCanvas, const QMargins &geometryMargins, const QSize &szItem)
    {
        qInfo() << "screen size" << szSceeen << "canvas" << szCanvas << "item size" << szItem;
        QMargins miniMargin = QMargins(2, 2, 2, 2);
        auto miniCellWidth = szItem.width() + miniMargin.left() + miniMargin.right();
        colCount = (szSceeen.width() - dockReserveArea.width()) / miniCellWidth;

        if (colCount < 1) {
            qCritical() << "!!!!! colCount is 0!!! set it 1 and set cellWidth to " << szCanvas.width();
            cellWidth = szCanvas.width();
            colCount = 1;
        } else {
            cellWidth = szCanvas.width() / colCount;
        }
        if (cellWidth < 1)
            cellWidth = 1;

        auto miniCellHeigh = szItem.height() + miniMargin.top() + miniMargin.bottom();
        rowCount = (szSceeen.height() - dockReserveArea.height()) / miniCellHeigh;
        if (rowCount < 1) {
            qCritical() << "!!!!! rowCount is 0!!! set it and set cellHeight to" << szCanvas.height();
            cellHeight = szCanvas.height();
            rowCount = 1;
        } else {
            cellHeight = szCanvas.height() / rowCount;
        }

        if (cellHeight < 1)
            cellHeight = 1;

        updateCellMargins(cellMargins, szItem, QSize(cellWidth, cellHeight));
        updateCellMargins(viewMargins, QSize(cellWidth * colCount, cellHeight * rowCount), szCanvas);
        viewMargins = geometryMargins + viewMargins;
    }

    Coordinate indexCoordinate(int index)
    {
        return Coordinate(index / rowCount, index % rowCount);
    }

    bool isVaildCoordinate(Coordinate coord)
    {
        return (coord.position().x() >= 0 && coord.position().x() < colCount)
                && (coord.position().y() >= 0 && coord.position().y() < rowCount);
    }

public:
    bool debug_show_grid = false;
    QString screenName;
    int screenNum;
    int rowCount = 0;
    int colCount = 0;
    int cellWidth = 0;
    int cellHeight = 0;
    QRect dockReserveArea { QRect(0, 0, 80, 80) };
    QMargins cellMargins { QMargins(2, 2, 2, 2) };
    QMargins viewMargins;
    QPoint dragTargetGrid { QPoint(-1, -1) };
};
#endif   // DEFAULTCANVASVIEW_P_H
