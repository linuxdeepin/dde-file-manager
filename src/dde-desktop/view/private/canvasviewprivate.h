// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "app/define.h"

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
#include <QEventLoop>
#include <dfilesystemwatcher.h>

#include "../../global/coorinate.h"
#include "../../dbus/dbusdock.h"
#include "../canvasgridview.h"
#include "desktopitemdelegate.h"

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
    CanvasViewPrivate(CanvasGridView *qq) : q(qq)
    {
        mousePressed = false;
        bReloadItem = false;
        dodgeDelayTimer.setInterval(200);

        touchTimer.setTimerType(Qt::PreciseTimer);
        QObject::connect(&touchTimer, &QTimer::timeout, &touchTimer, &QTimer::stop);

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
        bool byIcon = q->itemDelegate()->iconSizeMode() != DesktopItemDelegate::IconSizeMode::WordNum;
        qInfo() << "screen size" << szSceeen << "canvas" << szCanvas << "item size" << szItem
                 << "using word num" << !byIcon;

        auto reserveArea = byIcon ? dockReserveArea : QRect(0, 0, 0, 0);
        QMargins miniMargin = byIcon ? QMargins(2, 2, 2, 2)
                                      : QMargins(0, ICON_TOP_SPACE_DESKTOP, 0, 0);  // top margin is for icon top spacing

        auto miniCellWidth = szItem.width() + miniMargin.left() + miniMargin.right();
        colCount = (szSceeen.width() - reserveArea.width())/ miniCellWidth;

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
        rowCount = (szSceeen.height() - reserveArea.height())/ miniCellHeigh;
        if (rowCount < 1) {
            qCritical() << "!!!!! rowCount is 0!!! set it and set cellHeight to" << szCanvas.height();
            cellHeight = szCanvas.height();
            rowCount = 1;
        } else {
            cellHeight = szCanvas.height() / rowCount;
        }

        if (cellHeight < 1)
            cellHeight = 1;

        updateCellMargins(szItem, QSize(cellWidth, cellHeight));

        auto horizontalMargin = (szCanvas.width() - cellWidth * colCount);
        auto verticalMargin = (szCanvas.height() - cellHeight * rowCount);
        auto leftMargin = horizontalMargin / 2;
        auto rightMargin = horizontalMargin - leftMargin;
        auto topMargin = verticalMargin / 2;
        auto bottom = verticalMargin - topMargin;
        viewMargins = geometryMargins + QMargins(leftMargin, topMargin, rightMargin, bottom);
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

//    static QPair<int,QPoint> *lastMenuPos()
//    {
//        static QPair<int,QPoint> pos = qMakePair(0,QPoint(0,0));
//        return &pos;
//    }

public:
    QRect    dockReserveArea = QRect(0, 0, 80, 80);
    QMargins viewMargins;
    QMargins cellMargins = QMargins(2, 2, 2, 2);

    bool hideItems  = false;

    int rowCount = 0;
    int colCount = 0;
    int cellWidth = 0;
    int cellHeight = 0;

    QTimer              dodgeDelayTimer;
    QStringList         dodgeItems;
    bool                dodgeAnimationing   = false;
    double              dodgeDuration       = 0;
    GridCore            *dodgeTargetGrid = nullptr;
    bool                startDodge            = false;
    QPoint              dragTargetGrid   = QPoint(-1, -1);

    // currentCursorIndex is not the mouse, it's the position move by keybord
    QModelIndex         currentCursorIndex;
    //当前鼠标按下的index,currentCursorIndex是键盘移动选中的index
    //currentCursorIndex在使用上已经被多次误用了，所以这里使用新的变量，区分减小影响
    QModelIndex         m_currentMousePressIndex;

    //记录首次光标首次所在位置索引
    QModelIndex         m_oldCursorIndex;

    QPoint              lastMenuPos;
    QString             lastMenuNewFilepath;

    QPoint              lastPos;
    QPoint              beginPos{QPoint(-1, -1)};

    QItemSelection      beforeMoveSelection;
    bool                showSelectRect  = false;
    QRect               selectRect       = QRect(-1, -1, 1, 1);

    bool                mousePressed;

    QRect               canvasRect;
    CanvasViewHelper    *fileViewHelper = nullptr;

    bool                 bReloadItem;

    // secice system up
//    QTimer              *syncTimer          = nullptr;
//    qint64              lastRepaintTime     = 0;
    DAbstractFileWatcher  *filesystemWatcher  = nullptr;
    WaterMaskFrame *waterMaskFrame          = nullptr;

    //DBusDock            *dbusDock           = nullptr;
    QEventLoop          *menuLoop           = nullptr;

    // debug
    bool                _debug_log          = false;
    bool                _debug_show_grid    = false;
    bool                _debug_profiler     = false;

    // 用于实现触屏拖拽手指在屏幕上按下短时间200ms后响应
    QTimer touchTimer;
private:
    CanvasGridView *q;
};
