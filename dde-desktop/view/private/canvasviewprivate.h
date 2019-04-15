/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#pragma once

#include <QtGlobal>
#include <QModelIndex>
#include <QSize>
#include <QPoint>
#include <QRect>
#include <QMargins>
#include <QItemSelection>
#include <QDebug>
#include <QTimer>
#include <QLabel>

#include <dfilesystemwatcher.h>

#include "../../global/coorinate.h"
#include "../../dbus/dbusdock.h"
#include "../canvasgridview.h"

class QFrame;
class CanvasViewHelper;
class WaterMaskFrame;
class GridCore;
class DAbstractFileWatcher;

class CanvasViewPrivate
{
private:
    inline void updateCellMargins(const QSize &szItem, const QSize &szCell)
    {
        auto horizontalMargin = (szCell.width() - szItem.width());
        auto verticalMargin = (szCell.height() - szItem.height());
        auto leftMargin = horizontalMargin / 2;
        auto rightMargin = horizontalMargin - leftMargin;
        auto topMargin = verticalMargin / 2;
        auto bottom = verticalMargin - topMargin;

        cellMargins = QMargins(leftMargin, topMargin, rightMargin, bottom);
    }

public:
    CanvasViewPrivate()
    {
        cellMargins = QMargins(2, 2, 2, 2);
        selectRect = QRect(-1, -1, 1, 1);
        mousePressed = false;
        resortCount = 0;
        dodgeDelayTimer.setInterval(200);

        if (qgetenv("_DDE_DESKTOP_DEBUG_SHOW_GRID") == "TRUE") {
            _debug_log = true;
            _debug_show_grid = true;
        }

        if (qgetenv("_DDE_DESKTOP_DEBUG_PROFILER") == "TRUE") {
            _debug_profiler = true;
        }
    }

    void updateCanvasSize(const QSize &szSceeen, const QSize &szCanvas, const QMargins &geometryMargins, const QSize &szItem)
    {
        QMargins miniMargin = QMargins(2, 2, 2, 2);
        auto miniCellWidth = szItem.width() + miniMargin.left() + miniMargin.right();
        colCount = (szSceeen.width() - dockReserveArea.width()) / miniCellWidth;
        cellWidth = szCanvas.width() / colCount;

        auto miniCellHeigh = szItem.height() + miniMargin.top() + miniMargin.bottom();
//        qDebug() << szItem.height() << miniCellHeigh;
        rowCount = (szSceeen.height() - dockReserveArea.height()) / miniCellHeigh;

        cellHeight = szCanvas.height() / rowCount;
//        qDebug() << szSceeen.height() << dockReserveArea.height()
//                 << rowCount << cellHeight;
        updateCellMargins(szItem, QSize(cellWidth, cellHeight));

        auto horizontalMargin = (szCanvas.width() - cellWidth * colCount);
        auto verticalMargin = (szCanvas.height() - cellHeight * rowCount);
        auto leftMargin = horizontalMargin / 2;
        auto rightMargin = horizontalMargin - leftMargin;
        auto topMargin = verticalMargin / 2;
        auto bottom = verticalMargin - topMargin;
        viewMargins = geometryMargins + QMargins(leftMargin, topMargin, rightMargin, bottom);

//        qDebug() << "------------------------------";
//        qDebug() << miniCellWidth << miniCellHeigh;
//        qDebug() << szCanvas << colCount << rowCount;
//        qDebug() << viewMargins << cellWidth << cellHeight;
//        qDebug() << "------------------------------";
    }

    Coordinate indexCoordinate(int index)
    {
        return Coordinate(index / rowCount, index % rowCount);
    }

    int coordinateIndex(Coordinate coord)
    {
        return coord.position().x() * rowCount + coord.position().y();
    }

    bool isVaildCoordinate(Coordinate coord)
    {
        return (coord.position().x() >= 0 && coord.position().x() < colCount)
               && (coord.position().y() >= 0 && coord.position().y() < rowCount);
    }

public:
    QRect    dockReserveArea = QRect(0, 0, 80, 80);
    QMargins viewMargins;
    QMargins cellMargins;

    bool hideItems  = false;

    int rowCount;
    int colCount;
    int cellWidth;
    int cellHeight;

    QTimer              dodgeDelayTimer;
    QStringList         dodgeItems;
    bool                dodgeAnimationing   = false;
    double              dodgeDuration       = 0;
    GridCore            *dodgeTargetGrid;
    bool                startDodge            = false;
    QPoint              dragTargetGrid   = QPoint(-1, -1);

    // currentCursorIndex is not the mouse, it's the position move by keybord
    QModelIndex         currentCursorIndex;

    QPoint              lastMenuPos;
    QString             lastMenuNewFilepath;

    QPoint              lastPos;

    QItemSelection      beforeMoveSelection;
    bool                showSelectRect  = false;
    QRect               selectRect      = QRect();
    QFrame              *selectFrame    = nullptr;

    bool                mousePressed;

    QRect               canvasRect;
    CanvasViewHelper    *fileViewHelper = nullptr;

    int                 resortCount;

    // secice system up
//    QTimer              *syncTimer          = nullptr;
//    qint64              lastRepaintTime     = 0;
    DAbstractFileWatcher  *filesystemWatcher  = nullptr;
    WaterMaskFrame *waterMaskFrame          = nullptr;

    DBusDock            *dbusDock           = nullptr;

    // debug
    bool                _debug_log          = false;
    bool                _debug_show_grid    = false;
    bool                _debug_profiler     = false;

};
